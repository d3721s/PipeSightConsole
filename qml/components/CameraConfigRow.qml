import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

GroupBox {
    id: root

    property int channel: 0
    property int fieldHeight: 88
    property int compactFieldWidth: 220
    property int buttonWidth: 172

    readonly property var resolutionOptions: [
        "3840x2160", "2560x1440", "1920x1080", "1280x720", "704x576", "640x480"
    ]
    readonly property var fpsOptions: [60, 50, 30, 25, 20, 15, 10, 5]

    function reload() {
        const cfg = CameraViewModel.cameraConfig(root.channel)
        userField.text       = cfg.username || ""
        passwordField.text   = cfg.password || ""
        ipField.text         = cfg.ip || ""
        rtspPortField.text   = String(cfg.rtspPort  || 554)
        onvifPortField.text  = String(cfg.onvifPort || 80)
        chField.value        = cfg.channel || 1
        subBox.currentIndex  = (cfg.subtype === 1) ? 1 : 0

        const setCombo = function(box, options, value) {
            const idx = options.indexOf(value)
            box.currentIndex = idx >= 0 ? idx : 0
        }
        setCombo(mainResBox, root.resolutionOptions, cfg.mainResolution || "1920x1080")
        setCombo(subResBox,  root.resolutionOptions, cfg.subResolution  || "704x576")
        setCombo(mainFpsBox, root.fpsOptions, cfg.mainFps || 25)
        setCombo(subFpsBox,  root.fpsOptions, cfg.subFps  || 25)
    }

    Component.onCompleted: reload()

    Connections {
        target: CameraViewModel
        function onCameraConfigChanged(ch) { if (ch === root.channel) root.reload() }
    }

    function reportConfig() {
        CameraViewModel.configureCamera(
            root.channel,
            userField.text,
            passwordField.text,
            ipField.text,
            parseInt(rtspPortField.text)  || 554,
            parseInt(onvifPortField.text) || 80,
            chField.value,
            subBox.currentIndex,
            mainResBox.currentText,
            parseInt(mainFpsBox.currentText) || 25,
            subResBox.currentText,
            parseInt(subFpsBox.currentText) || 25)
    }

    ColumnLayout {
        width: parent.width
        spacing: 8

        // Row 1: 用户名 / 密码 / IP / RTSP端口 / ONVIF端口
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: qsTr("用户名") }
            TextField {
                id: userField
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                placeholderText: "username"
            }
            Label { text: qsTr("密码") }
            TextField {
                id: passwordField
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                echoMode: TextInput.Password
                placeholderText: "password"
            }
            Label { text: qsTr("IP") }
            TextField {
                id: ipField
                Layout.preferredWidth: root.compactFieldWidth + 60
                Layout.preferredHeight: root.fieldHeight
                placeholderText: "ip"
            }
            Label { text: qsTr("RTSP端口") }
            TextField {
                id: rtspPortField
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                text: "554"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 1; top: 65535 }
            }
            Label { text: qsTr("ONVIF端口") }
            TextField {
                id: onvifPortField
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                text: "80"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 1; top: 65535 }
            }
            Item { Layout.fillWidth: true }
        }

        // Row 2: 主码流分辨率 / 主码流FPS / 辅码流分辨率 / 辅码流FPS
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: qsTr("主码流分辨率") }
            ComboBox {
                id: mainResBox
                Layout.preferredWidth: root.compactFieldWidth + 50
                Layout.preferredHeight: root.fieldHeight
                model: root.resolutionOptions
            }
            Label { text: qsTr("主码流FPS") }
            ComboBox {
                id: mainFpsBox
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.fpsOptions
            }
            Label { text: qsTr("辅码流分辨率") }
            ComboBox {
                id: subResBox
                Layout.preferredWidth: root.compactFieldWidth + 50
                Layout.preferredHeight: root.fieldHeight
                model: root.resolutionOptions
            }
            Label { text: qsTr("辅码流FPS") }
            ComboBox {
                id: subFpsBox
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.fpsOptions
            }
            Item { Layout.fillWidth: true }
        }

        // Row 3: 通道 / 码流 / 应用
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: qsTr("通道") }
            SpinBox {
                id: chField
                from: 1; to: 64; value: 1
                editable: true
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
            }
            Label { text: qsTr("码流") }
            ComboBox {
                id: subBox
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: [qsTr("主码流"), qsTr("辅码流")]
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("应用")
                Layout.preferredWidth: root.buttonWidth
                Layout.preferredHeight: root.fieldHeight
                onClicked: root.reportConfig()
            }
        }
    }
}
