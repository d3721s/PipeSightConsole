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

    readonly property int resolutionFieldWidth: compactFieldWidth + 55
    readonly property int fpsFieldWidth: Math.max(86, compactFieldWidth - 18)
    readonly property int subCheckWidth: Math.max(116, compactFieldWidth)
    readonly property var resolutionOptions: [
        "3840 x 2160", "2560 x 1440", "1920 x 1080", "1280 x 720", "704 x 576", "640 x 480"
    ]
    readonly property var fpsOptions: [60, 50, 30, 25, 20, 15, 10, 5]

    function normalizeResolution(value, fallback) {
        const text = String(value || fallback)
        return text.replace(/\s*[xX]\s*/g, " x ").replace(/\s*\*\s*/g, " x ")
    }

    function setCombo(box, options, value) {
        const idx = options.indexOf(value)
        box.currentIndex = idx >= 0 ? idx : 0
    }

    function validSubtype(value) {
        const subtype = parseInt(value)
        return isNaN(subtype) ? 0 : Math.max(0, Math.min(2, subtype))
    }

    function reload() {
        const cfg = CameraViewModel.cameraConfig(root.channel)
        userField.text       = cfg.username || ""
        passwordField.text   = cfg.password || ""
        ipField.text         = cfg.ip || ""
        onvifPortField.text  = String(cfg.onvifPort || 80)
        chField.value        = cfg.channel || 1
        subBox.currentIndex  = validSubtype(cfg.subtype || 0)
        sub1EnabledBox.checked = cfg.sub1Enabled === undefined ? true : cfg.sub1Enabled
        sub2EnabledBox.checked = cfg.sub2Enabled === undefined ? false : cfg.sub2Enabled

        setCombo(mainResBox, root.resolutionOptions, normalizeResolution(cfg.mainResolution, "1920 x 1080"))
        setCombo(sub1ResBox, root.resolutionOptions, normalizeResolution(cfg.sub1Resolution, "704 x 576"))
        setCombo(sub2ResBox, root.resolutionOptions, normalizeResolution(cfg.sub2Resolution, "640 x 480"))
        setCombo(mainFpsBox, root.fpsOptions, cfg.mainFps || 25)
        setCombo(sub1FpsBox, root.fpsOptions, cfg.sub1Fps || 25)
        setCombo(sub2FpsBox, root.fpsOptions, cfg.sub2Fps || 25)
    }

    function readConfig() {
        root.reload()
        AppNotifier.info(qsTr("%1读取成功").arg(root.title))
    }

    Component.onCompleted: reload()

    Connections {
        target: CameraViewModel
        function onCameraConfigChanged(ch) { if (ch === root.channel) root.reload() }
    }

    AppConfirmDialog {
        id: confirmDialog
    }

    function reportConfig() {
        const ok = CameraViewModel.configureCamera(
            root.channel,
            userField.text,
            passwordField.text,
            ipField.text,
            parseInt(onvifPortField.text) || 80,
            chField.value,
            subBox.currentIndex,
            mainResBox.currentText,
            parseInt(mainFpsBox.currentText) || 25,
            sub1EnabledBox.checked,
            sub1ResBox.currentText,
            parseInt(sub1FpsBox.currentText) || 25,
            sub2EnabledBox.checked,
            sub2ResBox.currentText,
            parseInt(sub2FpsBox.currentText) || 25)
        if (ok)
            AppNotifier.info(qsTr("%1应用成功").arg(root.title))
        else
            AppNotifier.error(qsTr("%1应用失败").arg(root.title))
    }

    ColumnLayout {
        width: parent.width
        spacing: 8

        // Row 1: 用户名 / 密码 / IP / ONVIF端口
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

        // Row 2: 通道 / 码流选择 / 主码流设置
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
            Label { text: qsTr("码流选择") }
            ComboBox {
                id: subBox
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: [qsTr("主码流"), qsTr("辅码流1"), qsTr("辅码流2")]
            }
            Label { text: qsTr("主码流设置：") }
            Label { text: qsTr("W x H") }
            ComboBox {
                id: mainResBox
                Layout.preferredWidth: root.resolutionFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.resolutionOptions
            }
            Label { text: qsTr("FPS") }
            ComboBox {
                id: mainFpsBox
                Layout.preferredWidth: root.fpsFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.fpsOptions
            }
            Item { Layout.fillWidth: true }
        }

        // Row 3: 辅码流1 / 辅码流2 / 应用
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            CheckBox {
                id: sub1EnabledBox
                text: qsTr("辅码流1设置：")
                Layout.preferredWidth: Math.max(root.subCheckWidth, implicitWidth)
                Layout.preferredHeight: root.fieldHeight
            }
            Label { text: qsTr("W x H") }
            ComboBox {
                id: sub1ResBox
                enabled: sub1EnabledBox.checked
                Layout.preferredWidth: root.resolutionFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.resolutionOptions
            }
            Label { text: qsTr("FPS") }
            ComboBox {
                id: sub1FpsBox
                enabled: sub1EnabledBox.checked
                Layout.preferredWidth: root.fpsFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.fpsOptions
            }
            CheckBox {
                id: sub2EnabledBox
                text: qsTr("辅码流2设置：")
                Layout.preferredWidth: Math.max(root.subCheckWidth, implicitWidth)
                Layout.preferredHeight: root.fieldHeight
            }
            Label { text: qsTr("W x H") }
            ComboBox {
                id: sub2ResBox
                enabled: sub2EnabledBox.checked
                Layout.preferredWidth: root.resolutionFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.resolutionOptions
            }
            Label { text: qsTr("FPS") }
            ComboBox {
                id: sub2FpsBox
                enabled: sub2EnabledBox.checked
                Layout.preferredWidth: root.fpsFieldWidth
                Layout.preferredHeight: root.fieldHeight
                model: root.fpsOptions
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("读取")
                Layout.preferredWidth: root.buttonWidth
                Layout.preferredHeight: root.fieldHeight
                onClicked: root.readConfig()
            }
            Button {
                text: qsTr("应用")
                Layout.preferredWidth: root.buttonWidth
                Layout.preferredHeight: root.fieldHeight
                onClicked: confirmDialog.confirm(qsTr("确认应用%1？").arg(root.title),
                                                 function() { root.reportConfig() })
            }
        }
    }
}
