#!/usr/bin/env python3
"""Collect ONVIF SOAP responses into the lecheng result folders.

The conda environment for this script is expected to be:

    conda activate pipesight-onvif
    python tests/auto.py --host 192.168.71.21 --user admin

The script uses raw SOAP over HTTP Digest. onvif-zeep is installed in the
environment, but raw SOAP is used here because many cameras expose Media2,
DeviceIO, and Analytics operations that are not consistently covered by the
high-level Python ONVIF wrappers.
"""

from __future__ import annotations

import argparse
import getpass
import shutil
import sys
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Callable
from urllib.parse import urlparse
import xml.etree.ElementTree as ET

import requests
from requests.auth import HTTPDigestAuth


SOAP_NAMESPACES = {
    "s": "http://www.w3.org/2003/05/soap-envelope",
    "tds": "http://www.onvif.org/ver10/device/wsdl",
    "tt": "http://www.onvif.org/ver10/schema",
    "trt": "http://www.onvif.org/ver10/media/wsdl",
    "tr2": "http://www.onvif.org/ver20/media/wsdl",
    "tan": "http://www.onvif.org/ver20/analytics/wsdl",
    "tad": "http://www.onvif.org/ver10/analyticsdevice/wsdl",
    "timg": "http://www.onvif.org/ver20/imaging/wsdl",
    "tptz": "http://www.onvif.org/ver20/ptz/wsdl",
    "tev": "http://www.onvif.org/ver10/events/wsdl",
    "tmd": "http://www.onvif.org/ver10/deviceIO/wsdl",
}

SERVICE_NAMESPACES = {
    "http://www.onvif.org/ver10/device/wsdl": "Device_Management",
    "http://www.onvif.org/ver10/media/wsdl": "Media1",
    "http://www.onvif.org/ver20/media/wsdl": "Media2",
    "http://www.onvif.org/ver20/ptz/wsdl": "PTZ",
    "http://www.onvif.org/ver20/imaging/wsdl": "Imaging",
    "http://www.onvif.org/ver20/analytics/wsdl": "Analytics",
    "http://www.onvif.org/ver10/analyticsdevice/wsdl": "Analytics",
    "http://www.onvif.org/ver10/events/wsdl": "Events",
    "http://www.onvif.org/ver10/deviceIO/wsdl": "DeviceIO",
}

DEFAULT_SERVICE_PATHS = {
    "Device_Management": "/onvif/device_service",
    "Media1": "/onvif/media_service",
    "Media2": "/onvif/media2_service",
    "PTZ": "/onvif/ptz_service",
    "Imaging": "/onvif/imaging_service",
    "Analytics": "/onvif/analytics_service",
    "Events": "/onvif/event_service",
    "DeviceIO": "/onvif/deviceIO_service",
}


@dataclass(frozen=True)
class Operation:
    service: str
    outputs: tuple[str, ...]
    body: str | Callable[[dict[str, object]], str]
    requires: tuple[str, ...] = ()
    capture: str | None = None
    endpoint_key: str | None = None


def envelope(body: str) -> str:
    attrs = " ".join(f'xmlns:{prefix}="{uri}"' for prefix, uri in SOAP_NAMESPACES.items())
    return (
        '<?xml version="1.0" encoding="utf-8"?>\n'
        f"<s:Envelope {attrs}>\n"
        "  <s:Body>\n"
        f"{indent(body.strip(), 4)}\n"
        "  </s:Body>\n"
        "</s:Envelope>\n"
    )


def indent(text: str, spaces: int) -> str:
    prefix = " " * spaces
    return "\n".join(prefix + line if line.strip() else line for line in text.splitlines())


def local_name(tag: str) -> str:
    if "}" in tag:
        return tag.rsplit("}", 1)[1]
    return tag


def text_of_child(element: ET.Element, child_local_name: str) -> str | None:
    for child in list(element):
        if local_name(child.tag) == child_local_name and child.text:
            return child.text.strip()
    return None


def texts_by_local(root: ET.Element, names: set[str]) -> list[str]:
    values: list[str] = []
    for elem in root.iter():
        if local_name(elem.tag) in names and elem.text and elem.text.strip():
            values.append(elem.text.strip())
    return unique(values)


def token_attr(element: ET.Element) -> str | None:
    for key, value in element.attrib.items():
        if local_name(key).lower() == "token" and value:
            return value
    return None


def tokens_by_local(root: ET.Element, names: set[str]) -> list[str]:
    values: list[str] = []
    for elem in root.iter():
        if local_name(elem.tag) in names:
            token = token_attr(elem)
            if token:
                values.append(token)
    return unique(values)


def unique(values: list[str]) -> list[str]:
    seen: set[str] = set()
    result: list[str] = []
    for value in values:
        if value not in seen:
            seen.add(value)
            result.append(value)
    return result


def first(values: list[str]) -> str | None:
    return values[0] if values else None


def parse_xml(text: str) -> ET.Element | None:
    try:
        return ET.fromstring(text.encode("utf-8"))
    except ET.ParseError:
        return None


def add_tokens(state: dict[str, object], key: str, values: list[str]) -> None:
    if not values:
        return
    existing = list(state.get(key + "S", []))
    merged = unique(existing + values)
    state[key + "S"] = merged
    if key not in state or not state[key]:
        state[key] = merged[0]


def capture_from_response(state: dict[str, object], capture: str | None, root: ET.Element) -> None:
    if capture == "services":
        update_service_xaddrs(state, root)
        return

    if capture == "profiles":
        add_tokens(state, "PROFILE_TOKEN", tokens_by_local(root, {"Profiles", "Profile", "MediaProfile"}))
        add_tokens(state, "VIDEO_SOURCE_TOKEN", texts_by_local(root, {"SourceToken"}))
        add_tokens(state, "VIDEO_SOURCE_CONFIG_TOKEN", tokens_by_local(root, {"VideoSourceConfiguration"}))
        add_tokens(state, "VIDEO_ENCODER_CONFIG_TOKEN", tokens_by_local(root, {"VideoEncoderConfiguration"}))
        add_tokens(state, "PTZ_CONFIG_TOKEN", tokens_by_local(root, {"PTZConfiguration"}))
        add_tokens(state, "ANALYTICS_CONFIG_TOKEN", tokens_by_local(root, {"VideoAnalyticsConfiguration", "Analytics"}))
        return

    if capture == "video_sources":
        add_tokens(state, "VIDEO_SOURCE_TOKEN", tokens_by_local(root, {"VideoSources", "VideoSource"}))
        add_tokens(state, "VIDEO_SOURCE_TOKEN", [value for value in texts_by_local(root, {"Token"}) if value.startswith("VideoSource")])
        return

    if capture == "video_source_configs":
        add_tokens(state, "VIDEO_SOURCE_CONFIG_TOKEN", tokens_by_local(root, {"Configurations", "VideoSourceConfiguration"}))
        add_tokens(state, "VIDEO_SOURCE_TOKEN", texts_by_local(root, {"SourceToken"}))
        return

    if capture == "video_encoder_configs":
        add_tokens(state, "VIDEO_ENCODER_CONFIG_TOKEN", tokens_by_local(root, {"Configurations", "VideoEncoderConfiguration"}))
        return

    if capture == "ptz_configs":
        add_tokens(state, "PTZ_CONFIG_TOKEN", tokens_by_local(root, {"Configurations", "PTZConfiguration"}))
        return

    if capture == "analytics_configs":
        add_tokens(state, "ANALYTICS_CONFIG_TOKEN", tokens_by_local(root, {"Configurations", "VideoAnalyticsConfiguration", "Analytics"}))
        return

    if capture == "serial_ports":
        add_tokens(state, "SERIAL_PORT_TOKEN", tokens_by_local(root, {"SerialPort", "SerialPorts"}))
        add_tokens(state, "SERIAL_PORT_TOKEN", [value for value in texts_by_local(root, {"Token", "SerialPortToken"}) if value.startswith("Serial")])
        return

    if capture == "subscription":
        addresses = [value for value in texts_by_local(root, {"Address"}) if value.startswith(("http://", "https://"))]
        if addresses:
            state["SUBSCRIPTION_URL"] = addresses[0]


def update_service_xaddrs(state: dict[str, object], root: ET.Element) -> None:
    services = dict(state["services"])
    for service in root.iter():
        if local_name(service.tag) != "Service":
            continue
        namespace = text_of_child(service, "Namespace")
        xaddr = text_of_child(service, "XAddr")
        if not namespace or not xaddr:
            continue
        service_key = SERVICE_NAMESPACES.get(namespace)
        if service_key:
            services[service_key] = xaddr
    state["services"] = services


def require_text(state: dict[str, object], key: str) -> str:
    value = state.get(key)
    if not value:
        raise KeyError(key)
    return str(value)


def media2_stream_uri(state: dict[str, object]) -> str:
    return f"""
<tr2:GetStreamUri>
  <tr2:Protocol>RTSP</tr2:Protocol>
  <tr2:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</tr2:ProfileToken>
</tr2:GetStreamUri>
"""


def media2_snapshot_uri(state: dict[str, object]) -> str:
    return f"""
<tr2:GetSnapshotUri>
  <tr2:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</tr2:ProfileToken>
</tr2:GetSnapshotUri>
"""


def media2_video_source_options(state: dict[str, object]) -> str:
    return f"""
<tr2:GetVideoSourceConfigurationOptions>
  <tr2:ConfigurationToken>{require_text(state, "VIDEO_SOURCE_CONFIG_TOKEN")}</tr2:ConfigurationToken>
  <tr2:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</tr2:ProfileToken>
</tr2:GetVideoSourceConfigurationOptions>
"""


def media2_video_encoder_options(state: dict[str, object]) -> str:
    return f"""
<tr2:GetVideoEncoderConfigurationOptions>
  <tr2:ConfigurationToken>{require_text(state, "VIDEO_ENCODER_CONFIG_TOKEN")}</tr2:ConfigurationToken>
  <tr2:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</tr2:ProfileToken>
</tr2:GetVideoEncoderConfigurationOptions>
"""


def media2_osd_options(state: dict[str, object]) -> str:
    return f"""
<tr2:GetOSDOptions>
  <tr2:ConfigurationToken>{require_text(state, "VIDEO_SOURCE_CONFIG_TOKEN")}</tr2:ConfigurationToken>
</tr2:GetOSDOptions>
"""


def media1_stream_uri(state: dict[str, object]) -> str:
    return f"""
<trt:GetStreamUri>
  <trt:StreamSetup>
    <tt:Stream>RTP-Unicast</tt:Stream>
    <tt:Transport>
      <tt:Protocol>RTSP</tt:Protocol>
    </tt:Transport>
  </trt:StreamSetup>
  <trt:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</trt:ProfileToken>
</trt:GetStreamUri>
"""


def media1_snapshot_uri(state: dict[str, object]) -> str:
    return f"""
<trt:GetSnapshotUri>
  <trt:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</trt:ProfileToken>
</trt:GetSnapshotUri>
"""


def ptz_config_options(state: dict[str, object]) -> str:
    return f"""
<tptz:GetConfigurationOptions>
  <tptz:ConfigurationToken>{require_text(state, "PTZ_CONFIG_TOKEN")}</tptz:ConfigurationToken>
</tptz:GetConfigurationOptions>
"""


def ptz_status(state: dict[str, object]) -> str:
    return f"""
<tptz:GetStatus>
  <tptz:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</tptz:ProfileToken>
</tptz:GetStatus>
"""


def ptz_presets(state: dict[str, object]) -> str:
    return f"""
<tptz:GetPresets>
  <tptz:ProfileToken>{require_text(state, "PROFILE_TOKEN")}</tptz:ProfileToken>
</tptz:GetPresets>
"""


def imaging_with_video_source(operation: str, state: dict[str, object]) -> str:
    return f"""
<timg:{operation}>
  <timg:VideoSourceToken>{require_text(state, "VIDEO_SOURCE_TOKEN")}</timg:VideoSourceToken>
</timg:{operation}>
"""


def analytics_with_config(operation: str, state: dict[str, object]) -> str:
    return f"""
<tan:{operation}>
  <tan:ConfigurationToken>{require_text(state, "ANALYTICS_CONFIG_TOKEN")}</tan:ConfigurationToken>
</tan:{operation}>
"""


def pull_messages(state: dict[str, object]) -> str:
    return """
<tev:PullMessages>
  <tev:Timeout>PT5S</tev:Timeout>
  <tev:MessageLimit>10</tev:MessageLimit>
</tev:PullMessages>
"""


def serial_port(operation: str, state: dict[str, object]) -> str:
    return f"""
<tmd:{operation}>
  <tmd:SerialPortToken>{require_text(state, "SERIAL_PORT_TOKEN")}</tmd:SerialPortToken>
</tmd:{operation}>
"""


def operations() -> list[Operation]:
    return [
        Operation("Device_Management", ("Device_Management/Get_Services.xml", "Device_Management/Get_Services_Include_Capability.xml"), "<tds:GetServices><tds:IncludeCapability>true</tds:IncludeCapability></tds:GetServices>", capture="services"),
        Operation("Device_Management", ("Device_Management/Get_Service_Capabilities.xml",), "<tds:GetServiceCapabilities/>"),
        Operation("Device_Management", ("Device_Management/Get_Capabilities.xml", "Device_Management/Get_Capabilities_All.xml"), "<tds:GetCapabilities><tds:Category>All</tds:Category></tds:GetCapabilities>"),
        Operation("Device_Management", ("Device_Management/Device_Info.xml", "Device_Management/Get_Device_Information.xml"), "<tds:GetDeviceInformation/>"),
        Operation("Device_Management", ("Device_Management/Get_System_Date_And_Time.xml",), "<tds:GetSystemDateAndTime/>"),
        Operation("Device_Management", ("Device_Management/Get_Interfaces.xml", "Device_Management/Get_Network_Interfaces.xml"), "<tds:GetNetworkInterfaces/>"),
        Operation("Device_Management", ("Device_Management/Get_Network_Protocols.xml",), "<tds:GetNetworkProtocols/>"),
        Operation("Device_Management", ("Device_Management/Get_Scopes.xml",), "<tds:GetScopes/>"),
        Operation("Device_Management", ("Device_Management/Get_Hostname.xml",), "<tds:GetHostname/>"),
        Operation("Device_Management", ("Device_Management/Get_Users.xml",), "<tds:GetUsers/>"),
        Operation("Device_Management", ("Device_Management/Get_System_Uris.xml",), "<tds:GetSystemUris/>"),
        Operation("Media2", ("Media2/Get_Service_Capabilities.xml",), "<tr2:GetServiceCapabilities/>"),
        Operation("Media2", ("Media2/Get_Profiles.xml",), "<tr2:GetProfiles/>", capture="profiles"),
        Operation("Media2", ("Media2/Get_Video_Source_Configurations.xml",), "<tr2:GetVideoSourceConfigurations/>", capture="video_source_configs"),
        Operation("Media2", ("Media2/Get_Video_Encoder_Configurations.xml",), "<tr2:GetVideoEncoderConfigurations/>", capture="video_encoder_configs"),
        Operation("Media2", ("Media2/Get_Audio_Source_Configurations.xml",), "<tr2:GetAudioSourceConfigurations/>"),
        Operation("Media2", ("Media2/Get_Audio_Encoder_Configurations.xml",), "<tr2:GetAudioEncoderConfigurations/>"),
        Operation("Media2", ("Media2/Get_Audio_Output_Configurations.xml",), "<tr2:GetAudioOutputConfigurations/>"),
        Operation("Media2", ("Media2/Get_Audio_Decoder_Configurations.xml",), "<tr2:GetAudioDecoderConfigurations/>"),
        Operation("Media2", ("Media2/Get_Metadata_Configurations.xml",), "<tr2:GetMetadataConfigurations/>"),
        Operation("Media2", ("Media2/Get_Analytics_Configurations.xml",), "<tr2:GetAnalyticsConfigurations/>", capture="analytics_configs"),
        Operation("Media2", ("Media2/Get_OSDs.xml",), "<tr2:GetOSDs/>"),
        Operation("Media1", ("Media1/Get_Service_Capabilities.xml",), "<trt:GetServiceCapabilities/>"),
        Operation("Media1", ("Media1/Get_Profiles.xml",), "<trt:GetProfiles/>", capture="profiles"),
        Operation("Media1", ("Media1/Get_Video_Sources.xml",), "<trt:GetVideoSources/>", capture="video_sources"),
        Operation("Media1", ("Media1/Get_Audio_Sources.xml",), "<trt:GetAudioSources/>"),
        Operation("Media1", ("Media1/Get_Audio_Outputs.xml",), "<trt:GetAudioOutputs/>"),
        Operation("Media1", ("Media1/Get_Video_Source_Configurations.xml",), "<trt:GetVideoSourceConfigurations/>", capture="video_source_configs"),
        Operation("Media1", ("Media1/Get_Video_Encoder_Configurations.xml",), "<trt:GetVideoEncoderConfigurations/>", capture="video_encoder_configs"),
        Operation("Media1", ("Media1/Get_Audio_Source_Configurations.xml",), "<trt:GetAudioSourceConfigurations/>"),
        Operation("Media1", ("Media1/Get_Audio_Encoder_Configurations.xml",), "<trt:GetAudioEncoderConfigurations/>"),
        Operation("Media1", ("Media1/Get_Metadata_Configurations.xml",), "<trt:GetMetadataConfigurations/>"),
        Operation("Media1", ("Media1/Get_Video_Analytics_Configurations.xml",), "<trt:GetVideoAnalyticsConfigurations/>", capture="analytics_configs"),
        Operation("Media1", ("Media1/Get_OSDs.xml",), "<trt:GetOSDs/>"),
        Operation("PTZ", ("PTZ/Get_Service_Capabilities.xml",), "<tptz:GetServiceCapabilities/>"),
        Operation("PTZ", ("PTZ/Get_Nodes.xml",), "<tptz:GetNodes/>"),
        Operation("PTZ", ("PTZ/Get_Configurations.xml",), "<tptz:GetConfigurations/>", capture="ptz_configs"),
        Operation("Imaging", ("Imaging/Get_Service_Capabilities.xml",), "<timg:GetServiceCapabilities/>"),
        Operation("Analytics", ("Analytics/Get_Service_Capabilities.xml",), "<tan:GetServiceCapabilities/>"),
        Operation("Events", ("Events/Get_Service_Capabilities.xml",), "<tev:GetServiceCapabilities/>"),
        Operation("Events", ("Events/Get_Event_Properties.xml",), "<tev:GetEventProperties/>"),
        Operation("Events", ("Events/Create_Pull_Point_Subscription.xml",), "<tev:CreatePullPointSubscription><tev:InitialTerminationTime>PT1M</tev:InitialTerminationTime></tev:CreatePullPointSubscription>", capture="subscription"),
        Operation("DeviceIO", ("DeviceIO/Get_Service_Capabilities.xml",), "<tmd:GetServiceCapabilities/>"),
        Operation("DeviceIO", ("DeviceIO/Get_Video_Sources.xml",), "<tmd:GetVideoSources/>", capture="video_sources"),
        Operation("DeviceIO", ("DeviceIO/Get_Video_Outputs.xml",), "<tmd:GetVideoOutputs/>"),
        Operation("DeviceIO", ("DeviceIO/Get_Audio_Sources.xml",), "<tmd:GetAudioSources/>"),
        Operation("DeviceIO", ("DeviceIO/Get_Audio_Outputs.xml",), "<tmd:GetAudioOutputs/>"),
        Operation("DeviceIO", ("DeviceIO/Get_Relay_Outputs.xml",), "<tmd:GetRelayOutputs/>"),
        Operation("DeviceIO", ("DeviceIO/Get_Digital_Inputs.xml",), "<tmd:GetDigitalInputs/>"),
        Operation("DeviceIO", ("DeviceIO/Get_Serial_Ports.xml",), "<tmd:GetSerialPorts/>", capture="serial_ports"),
        Operation("Media2", ("Media2/Get_Stream_Uri.xml", "Media2/Get_Stream_Uri_RTSP.xml"), media2_stream_uri, requires=("PROFILE_TOKEN",)),
        Operation("Media2", ("Media2/Get_Snapshot_Uri.xml",), media2_snapshot_uri, requires=("PROFILE_TOKEN",)),
        Operation("Media2", ("Media2/Get_Video_Source_Configuration_Options.xml",), media2_video_source_options, requires=("VIDEO_SOURCE_CONFIG_TOKEN", "PROFILE_TOKEN")),
        Operation("Media2", ("Media2/Get_Video_Encoder_Configuration_Options.xml",), media2_video_encoder_options, requires=("VIDEO_ENCODER_CONFIG_TOKEN", "PROFILE_TOKEN")),
        Operation("Media2", ("Media2/Get_OSD_Options.xml",), media2_osd_options, requires=("VIDEO_SOURCE_CONFIG_TOKEN",)),
        Operation("Media1", ("Media1/Get_Stream_Uri.xml",), media1_stream_uri, requires=("PROFILE_TOKEN",)),
        Operation("Media1", ("Media1/Get_Snapshot_Uri.xml",), media1_snapshot_uri, requires=("PROFILE_TOKEN",)),
        Operation("PTZ", ("PTZ/Get_Configuration_Options.xml",), ptz_config_options, requires=("PTZ_CONFIG_TOKEN",)),
        Operation("PTZ", ("PTZ/Get_Status.xml",), ptz_status, requires=("PROFILE_TOKEN",)),
        Operation("PTZ", ("PTZ/Get_Presets.xml",), ptz_presets, requires=("PROFILE_TOKEN",)),
        Operation("Imaging", ("Imaging/Get_Imaging_Settings.xml",), lambda state: imaging_with_video_source("GetImagingSettings", state), requires=("VIDEO_SOURCE_TOKEN",)),
        Operation("Imaging", ("Imaging/Get_Options.xml",), lambda state: imaging_with_video_source("GetOptions", state), requires=("VIDEO_SOURCE_TOKEN",)),
        Operation("Imaging", ("Imaging/Get_Move_Options.xml",), lambda state: imaging_with_video_source("GetMoveOptions", state), requires=("VIDEO_SOURCE_TOKEN",)),
        Operation("Imaging", ("Imaging/Get_Status.xml",), lambda state: imaging_with_video_source("GetStatus", state), requires=("VIDEO_SOURCE_TOKEN",)),
        Operation("Analytics", ("Analytics/Get_Supported_Rules.xml",), lambda state: analytics_with_config("GetSupportedRules", state), requires=("ANALYTICS_CONFIG_TOKEN",)),
        Operation("Analytics", ("Analytics/Get_Rules.xml",), lambda state: analytics_with_config("GetRules", state), requires=("ANALYTICS_CONFIG_TOKEN",)),
        Operation("Analytics", ("Analytics/Get_Supported_Analytics_Modules.xml",), lambda state: analytics_with_config("GetSupportedAnalyticsModules", state), requires=("ANALYTICS_CONFIG_TOKEN",)),
        Operation("Analytics", ("Analytics/Get_Analytics_Modules.xml",), lambda state: analytics_with_config("GetAnalyticsModules", state), requires=("ANALYTICS_CONFIG_TOKEN",)),
        Operation("Events", ("Events/Pull_Messages.xml",), pull_messages, requires=("SUBSCRIPTION_URL",), endpoint_key="SUBSCRIPTION_URL"),
        Operation("DeviceIO", ("DeviceIO/Get_Serial_Port_Configuration.xml",), lambda state: serial_port("GetSerialPortConfiguration", state), requires=("SERIAL_PORT_TOKEN",)),
        Operation("DeviceIO", ("DeviceIO/Get_Serial_Port_Configuration_Options.xml",), lambda state: serial_port("GetSerialPortConfigurationOptions", state), requires=("SERIAL_PORT_TOKEN",)),
    ]


def initial_state(host: str, port: int, scheme: str) -> dict[str, object]:
    base = f"{scheme}://{host}:{port}" if port else f"{scheme}://{host}"
    services = {key: base + path for key, path in DEFAULT_SERVICE_PATHS.items()}
    return {
        "services": services,
        "PROFILE_TOKENS": [],
        "VIDEO_SOURCE_TOKENS": [],
        "VIDEO_SOURCE_CONFIG_TOKENS": [],
        "VIDEO_ENCODER_CONFIG_TOKENS": [],
        "PTZ_CONFIG_TOKENS": [],
        "ANALYTICS_CONFIG_TOKENS": [],
        "SERIAL_PORT_TOKENS": [],
    }


def request_operation(session: requests.Session, url: str, body: str, timeout: float) -> tuple[requests.Response | None, str | None]:
    soap = envelope(body)
    try:
        response = session.post(
            url,
            data=soap.encode("utf-8"),
            headers={"Content-Type": "application/soap+xml; charset=utf-8"},
            timeout=timeout,
        )
        return response, None
    except requests.RequestException as exc:
        return None, f"{type(exc).__name__}: {exc}"


def render_transaction(url: str, body: str, response: requests.Response | None, error: str | None) -> str:
    parsed = urlparse(url)
    target = parsed.path or "/"
    if parsed.query:
        target += "?" + parsed.query

    request_text = [
        "Request:",
        "",
        f"POST {target} HTTP/1.1",
        f"Host: {parsed.netloc}",
        "Content-Type: application/soap+xml; charset=utf-8",
        "",
        envelope(body).rstrip(),
        "",
    ]

    if error:
        response_text = ["Response:", "", f"ERROR: {error}", ""]
    elif response is None:
        response_text = ["Response:", "", "ERROR: no response", ""]
    else:
        lines = [
            "Response:",
            "",
            f"HTTP/1.1 {response.status_code} {response.reason}",
        ]
        for key, value in response.headers.items():
            if key.lower() in {"content-type", "content-length", "connection", "date", "server"}:
                lines.append(f"{key}: {value}")
        lines.extend(["", response.text.rstrip(), ""])
        response_text = lines

    return "\n".join(request_text + response_text)


def render_skipped(op: Operation, state: dict[str, object]) -> str:
    missing = [key for key in op.requires if not state.get(key)]
    body_text = body_for_operation(op, state, allow_missing=True)
    return "\n".join(
        [
            "Skipped:",
            "",
            f"Service: {op.service}",
            f"Missing token(s): {', '.join(missing)}",
            "",
            "Known tokens:",
            f"PROFILE_TOKEN: {state.get('PROFILE_TOKEN', '')}",
            f"VIDEO_SOURCE_TOKEN: {state.get('VIDEO_SOURCE_TOKEN', '')}",
            f"VIDEO_SOURCE_CONFIG_TOKEN: {state.get('VIDEO_SOURCE_CONFIG_TOKEN', '')}",
            f"VIDEO_ENCODER_CONFIG_TOKEN: {state.get('VIDEO_ENCODER_CONFIG_TOKEN', '')}",
            f"PTZ_CONFIG_TOKEN: {state.get('PTZ_CONFIG_TOKEN', '')}",
            f"ANALYTICS_CONFIG_TOKEN: {state.get('ANALYTICS_CONFIG_TOKEN', '')}",
            f"SERIAL_PORT_TOKEN: {state.get('SERIAL_PORT_TOKEN', '')}",
            f"SUBSCRIPTION_URL: {state.get('SUBSCRIPTION_URL', '')}",
            "",
            "Request template:",
            "",
            body_text.strip(),
            "",
        ]
    )


def body_for_operation(op: Operation, state: dict[str, object], allow_missing: bool = False) -> str:
    if isinstance(op.body, str):
        return op.body
    if allow_missing:
        fake = dict(state)
        for key in op.requires:
            fake.setdefault(key, "MISSING_" + key)
        try:
            return op.body(fake)
        except Exception:
            return "<!-- request needs tokens from earlier responses -->"
    return op.body(state)


def write_outputs(args: argparse.Namespace, outputs: tuple[str, ...], content: str) -> None:
    for output in outputs:
        path = Path(args.out) / output
        path.parent.mkdir(parents=True, exist_ok=True)
        if args.skip_existing and path.exists() and path.stat().st_size > 0:
            continue
        if args.backup and path.exists() and path.stat().st_size > 0:
            stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup = path.with_name(path.name + f".{stamp}.bak")
            if not backup.exists():
                shutil.copy2(path, backup)
        path.write_text(content, encoding="utf-8")


def run(args: argparse.Namespace) -> int:
    password = args.password
    if password is None and args.user:
        password = getpass.getpass("ONVIF password: ")

    session = requests.Session()
    if args.user:
        session.auth = HTTPDigestAuth(args.user, password or "")

    state = initial_state(args.host, args.port, args.scheme)
    if args.serial_token:
        add_tokens(state, "SERIAL_PORT_TOKEN", [args.serial_token])
    selected = {name.lower() for name in args.only}
    ok = failed = skipped = 0

    for op in operations():
        if selected and op.service.lower() not in selected:
            continue

        if any(not state.get(key) for key in op.requires):
            write_outputs(args, op.outputs, render_skipped(op, state))
            print(f"SKIP {op.service}: {', '.join(op.outputs)}")
            skipped += 1
            continue

        try:
            body = body_for_operation(op, state)
        except KeyError as exc:
            write_outputs(args, op.outputs, render_skipped(op, state))
            print(f"SKIP {op.service}: missing {exc}")
            skipped += 1
            continue

        url = str(state.get(op.endpoint_key or "", "")) if op.endpoint_key else dict(state["services"])[op.service]
        response, error = request_operation(session, url, body, args.timeout)
        write_outputs(args, op.outputs, render_transaction(url, body, response, error))

        if error or response is None:
            print(f"FAIL {op.service}: {op.outputs[0]} -> {error}")
            failed += 1
            continue

        root = parse_xml(response.text)
        if root is not None:
            capture_from_response(state, op.capture, root)

        if 200 <= response.status_code < 300:
            print(f"OK   {op.service}: {op.outputs[0]}")
            ok += 1
        else:
            print(f"FAIL {op.service}: {op.outputs[0]} -> HTTP {response.status_code}")
            failed += 1

    print("")
    print(f"Done. ok={ok}, failed={failed}, skipped={skipped}")
    print("Tokens:")
    for key in [
        "PROFILE_TOKEN",
        "VIDEO_SOURCE_TOKEN",
        "VIDEO_SOURCE_CONFIG_TOKEN",
        "VIDEO_ENCODER_CONFIG_TOKEN",
        "PTZ_CONFIG_TOKEN",
        "ANALYTICS_CONFIG_TOKEN",
        "SERIAL_PORT_TOKEN",
        "SUBSCRIPTION_URL",
    ]:
        print(f"  {key}={state.get(key, '')}")
    return 1 if failed else 0


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Collect ONVIF SOAP responses into lecheng/*.xml files.")
    parser.add_argument("--host", default="192.168.71.21", help="Camera IP or host name.")
    parser.add_argument("--port", type=int, default=80, help="Camera HTTP port. Use 0 to omit the port in URLs.")
    parser.add_argument("--scheme", default="http", choices=("http", "https"), help="URL scheme.")
    parser.add_argument("--user", default="admin", help="ONVIF username. Empty string disables auth.")
    parser.add_argument("--password", help="ONVIF password. If omitted, prompt securely.")
    parser.add_argument("--out", default="lecheng", help="Output folder.")
    parser.add_argument("--timeout", type=float, default=8.0, help="HTTP timeout in seconds.")
    parser.add_argument("--serial-token", help="Fallback serial port token, for example SerialPort000.")
    parser.add_argument("--skip-existing", action="store_true", help="Do not overwrite non-empty output files.")
    parser.add_argument("--no-backup", dest="backup", action="store_false", help="Do not create .bak files before overwriting.")
    parser.add_argument("--only", action="append", default=[], help="Run only one service folder. Can be repeated.")
    parser.set_defaults(backup=True)
    return parser.parse_args(argv)


def main() -> int:
    return run(parse_args(sys.argv[1:]))


if __name__ == "__main__":
    raise SystemExit(main())
