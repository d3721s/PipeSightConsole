import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import PipeSightConsole

Item {
    id: panel
    readonly property int fieldHeight: 44
    readonly property int cameraFieldWidth: 150
    readonly property int compactFieldWidth: 110
    readonly property int buttonWidth: 86
    property string storagePathDraft: ConfigViewModel.storagePath

    function localPathFromUrl(url) {
        const text = decodeURIComponent(String(url))
        if (text.indexOf("file:///") === 0)
            return Qt.platform.os === "windows" ? text.substring(8) : text.substring(7)
        return text
    }

    function readStereoParams() {
        exp.value = Number(ConfigViewModel.stereoParam("exposure"))
        const wb = String(ConfigViewModel.stereoParam("whiteBalance"))
        const wbIndex = ["Auto", "Daylight", "Cloudy", "Tungsten"].indexOf(wb)
        wbBox.currentIndex = wbIndex >= 0 ? wbIndex : 0
        syncExposure.checked = Boolean(ConfigViewModel.stereoParam("sync"))
    }

    function applyStereoParams() {
        ConfigViewModel.setStereoParam("exposure", exp.value)
        ConfigViewModel.setStereoParam("whiteBalance", wbBox.currentText)
        ConfigViewModel.setStereoParam("sync", syncExposure.checked)
    }

    function readRadarParams() {
        scanHzBox.value = Number(ConfigViewModel.radarParam("scanHz"))
        angleDegBox.value = Number(ConfigViewModel.radarParam("angleDeg"))
        rangeMBox.value = Number(ConfigViewModel.radarParam("rangeM"))
    }

    function applyRadarParams() {
        ConfigViewModel.setRadarParam("scanHz", scanHzBox.value)
        ConfigViewModel.setRadarParam("angleDeg", angleDegBox.value)
        ConfigViewModel.setRadarParam("rangeM", rangeMBox.value)
    }

    Component.onCompleted: {
        readStereoParams()
        readRadarParams()
    }

    FolderDialog {
        id: storageFolderDialog
        title: qsTr("选择录像存储路径")
        currentFolder: storagePathDraft.length ? "file:///" + storagePathDraft : StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        onAccepted: storagePathDraft = panel.localPathFromUrl(selectedFolder)
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 4
        contentWidth: width

        ColumnLayout {
            width: parent.width
            spacing: 10

            // ---- Camera 1 (前摄) ----
            CameraConfigRow {
                Layout.fillWidth: true
                channel: 0
                title: qsTr("前摄配置")
                fieldHeight: panel.fieldHeight
                compactFieldWidth: panel.compactFieldWidth
                buttonWidth: panel.buttonWidth
            }

            // ---- Camera 2 (后摄) ----
            CameraConfigRow {
                Layout.fillWidth: true
                channel: 1
                title: qsTr("后摄配置")
                fieldHeight: panel.fieldHeight
                compactFieldWidth: panel.compactFieldWidth
                buttonWidth: panel.buttonWidth
            }

            // ---- Recording ----
            GroupBox {
                title: qsTr("录像配置")
                Layout.fillWidth: true
                RowLayout {
                    width: parent.width
                    spacing: 10

                    Label { text: qsTr("分段时长 (分钟)") }
                    SpinBox {
                        id: segmentMinutesBox
                        from: 1; to: 240
                        value: ConfigViewModel.segmentMinutes
                        Layout.preferredWidth: panel.compactFieldWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onValueModified: ConfigViewModel.segmentMinutes = value
                    }
                    CheckBox {
                        id: cyclicRecordSwitch
                        text: qsTr("循环录像")
                        checked: ConfigViewModel.cyclicRecord
                        onToggled: ConfigViewModel.cyclicRecord = checked
                    }
                    Label { text: qsTr("存储路径") }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: panel.fieldHeight
                        color: "transparent"
                        border.color: "#5b5b64"
                        border.width: 1
                        radius: 4
                        Label {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideMiddle
                            text: panel.storagePathDraft
                        }
                    }
                    Button {
                        text: qsTr("选择")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: storageFolderDialog.open()
                    }
                    Button {
                        text: qsTr("应用")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: {
                            ConfigViewModel.segmentMinutes = segmentMinutesBox.value
                            ConfigViewModel.cyclicRecord = cyclicRecordSwitch.checked
                            ConfigViewModel.storagePath = panel.storagePathDraft
                        }
                    }
                }
            }

            OsdPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 152
            }

            // ---- Stereo ----
            GroupBox {
                title: qsTr("双目相机配置")
                Layout.fillWidth: true
                RowLayout {
                    width: parent.width
                    spacing: 10

                    Label { text: qsTr("曝光") }
                    Slider { id: exp; from: 0; to: 100; value: 50; Layout.preferredWidth: 320
                    }
                    Label { text: qsTr("白平衡") }
                    ComboBox {
                        id: wbBox
                        Layout.preferredWidth: 180
                        Layout.preferredHeight: panel.fieldHeight
                        model: ["Auto", "Daylight", "Cloudy", "Tungsten"]
                    }
                    CheckBox {
                        id: syncExposure
                        text: qsTr("同步曝光")
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: qsTr("读取")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: panel.readStereoParams()
                    }
                    Button {
                        text: qsTr("应用")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: panel.applyStereoParams()
                    }
                }
            }

            // ---- Radar ----
            GroupBox {
                title: qsTr("激光雷达配置")
                Layout.fillWidth: true
                RowLayout {
                    width: parent.width
                    spacing: 10

                    Label { text: qsTr("扫描频率 (Hz)") }
                    SpinBox { id: scanHzBox; from: 1; to: 50; value: 10; Layout.preferredWidth: panel.compactFieldWidth; Layout.preferredHeight: panel.fieldHeight
                    }
                    Label { text: qsTr("扫描角度 (°)") }
                    SpinBox { id: angleDegBox; from: 30; to: 360; value: 270; Layout.preferredWidth: panel.compactFieldWidth; Layout.preferredHeight: panel.fieldHeight
                    }
                    Label { text: qsTr("距离范围 (m)") }
                    SpinBox { id: rangeMBox; from: 1; to: 100; value: 30; Layout.preferredWidth: panel.compactFieldWidth; Layout.preferredHeight: panel.fieldHeight
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: qsTr("读取")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: panel.readRadarParams()
                    }
                    Button {
                        text: qsTr("应用")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: panel.applyRadarParams()
                    }
                }
            }

        }
    }
}
