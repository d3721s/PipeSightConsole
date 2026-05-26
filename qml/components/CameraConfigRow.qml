import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

GroupBox {
    id: root

    property int channel: 0
    property int fieldHeight: 44
    property int compactFieldWidth: 110
    property int buttonWidth: 86

    function reload() {
        const cfg = CameraViewModel.cameraConfig(root.channel)
        userField.text     = cfg.username || ""
        passwordField.text = cfg.password || ""
        ipField.text       = cfg.ip || ""
        portField.value    = cfg.port    || 554
        chField.value      = cfg.channel || 1
        subBox.currentIndex = (cfg.subtype === 1) ? 1 : 0
        urlPreview.text = CameraViewModel.cameraStreamUrl(root.channel)
    }

    Component.onCompleted: reload()

    Connections {
        target: CameraViewModel
        function onCameraConfigChanged(ch) { if (ch === root.channel) root.reload() }
    }

    ColumnLayout {
        width: parent.width
        spacing: 8

        GridLayout {
            Layout.fillWidth: true
            columns: 8
            columnSpacing: 8
            rowSpacing: 6

            Label { text: qsTr("用户名") }
            TextField {
                id: userField
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                placeholderText: "admin"
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
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
                placeholderText: "192.168.1.10"
            }
            Label { text: qsTr("端口") }
            SpinBox {
                id: portField
                from: 1; to: 65535; value: 554
                editable: true
                Layout.preferredWidth: root.compactFieldWidth
                Layout.preferredHeight: root.fieldHeight
            }

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
                Layout.preferredWidth: root.compactFieldWidth + 20
                Layout.preferredHeight: root.fieldHeight
                model: [qsTr("主码流 (subtype=0)"), qsTr("辅码流 (subtype=1)")]
            }
            Item { Layout.columnSpan: 3; Layout.fillWidth: true }
            Button {
                text: qsTr("应用")
                Layout.preferredWidth: root.buttonWidth
                Layout.preferredHeight: root.fieldHeight
                onClicked: CameraViewModel.configureCamera(
                    root.channel,
                    userField.text,
                    passwordField.text,
                    ipField.text,
                    portField.value,
                    chField.value,
                    subBox.currentIndex)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Label {
                text: qsTr("RTSP")
                color: "#888"
            }
            Label {
                id: urlPreview
                Layout.fillWidth: true
                color: "#aaa"
                elide: Text.ElideRight
                font.family: "monospace"
                text: ""
            }
        }
    }
}
