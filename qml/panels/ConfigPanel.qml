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

    function notifyResult(ok, successMessage, failureMessage) {
        if (ok)
            AppNotifier.info(successMessage)
        else
            AppNotifier.error(failureMessage)
    }

    function loadLocalStereoParams() {
        exp.value = Number(ConfigViewModel.stereoParam("exposure"))
        const wb = String(ConfigViewModel.stereoParam("whiteBalance"))
        const wbIndex = ["Auto", "Daylight", "Cloudy", "Tungsten"].indexOf(wb)
        wbBox.currentIndex = wbIndex >= 0 ? wbIndex : 0
        syncExposure.checked = Boolean(ConfigViewModel.stereoParam("sync"))
    }

    function readStereoParams() {
        const ok = ConfigViewModel.readStereoParams()
        if (ok)
            loadLocalStereoParams()
        notifyResult(ok, qsTr("深度相机参数读取完成"), qsTr("深度相机参数读取失败：后端未实现"))
    }

    function applyStereoParams() {
        const ok = ConfigViewModel.applyStereoParams(exp.value, wbBox.currentText, syncExposure.checked)
        notifyResult(ok, qsTr("深度相机参数应用完成"), qsTr("深度相机参数应用失败：后端未实现"))
    }

    function loadLocalRadarParams() {
        scanHzBox.value = Number(ConfigViewModel.radarParam("scanHz"))
        angleDegBox.value = Number(ConfigViewModel.radarParam("angleDeg"))
        rangeMBox.value = Number(ConfigViewModel.radarParam("rangeM"))
    }

    function readRadarParams() {
        const ok = ConfigViewModel.readRadarParams()
        if (ok)
            loadLocalRadarParams()
        notifyResult(ok, qsTr("激光雷达参数读取完成"), qsTr("激光雷达参数读取失败：后端未实现"))
    }

    function applyRadarParams() {
        const ok = ConfigViewModel.applyRadarParams(scanHzBox.value, angleDegBox.value, rangeMBox.value)
        notifyResult(ok, qsTr("激光雷达参数应用完成"), qsTr("激光雷达参数应用失败：后端未实现"))
    }

    function applyRecordingConfig() {
        const ok = ConfigViewModel.applyRecordingConfig(
            segmentMinutesBox.value,
            cyclicRecordSwitch.checked,
            panel.storagePathDraft)
        if (ok)
            panel.storagePathDraft = ConfigViewModel.storagePath
        panel.notifyResult(ok, qsTr("录像配置应用完成"), qsTr("录像配置应用失败：存储路径不可用"))
    }

    Component.onCompleted: {
        loadLocalStereoParams()
        loadLocalRadarParams()
    }

    AppConfirmDialog {
        id: confirmDialog
    }

    FolderDialog {
        id: storageFolderDialog
        title: qsTr("选择录像存储路径")
        currentFolder: storagePathDraft.length ? "file:///" + storagePathDraft : StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        onAccepted: storagePathDraft = panel.localPathFromUrl(selectedFolder)
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 4
        contentWidth: width
        contentHeight: contentLayout.implicitHeight
        interactive: contentHeight > height
        boundsBehavior: interactive ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
        clip: true
        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AlwaysOff }
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        ColumnLayout {
            id: contentLayout
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
                title: qsTr("摄录配置")
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
                    }
                    CheckBox {
                        id: cyclicRecordSwitch
                        text: qsTr("循环录像")
                        checked: ConfigViewModel.cyclicRecord
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
                        onClicked: confirmDialog.confirm(qsTr("确认应用摄录配置？"),
                                                         function() { panel.applyRecordingConfig() })
                    }
                }
            }

            OsdPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 152
            }

            // ---- Stereo ----
            GroupBox {
                title: qsTr("深度相机配置")
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
                        onClicked: confirmDialog.confirm(qsTr("确认应用深度相机参数？"),
                                                         function() { panel.applyStereoParams() })
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
                        onClicked: confirmDialog.confirm(qsTr("确认应用激光雷达参数？"),
                                                         function() { panel.applyRadarParams() })
                    }
                }
            }

        }
    }
}
