import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

Item {
    id: panel
    readonly property int controlButtonHeight: 48

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 4

        // Live video on the left — keep a fixed aspect ratio (16:9) and
        // scale to the largest size that fits the available cell.
        Item {
            id: videoFrame
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 0

            readonly property real aspect: 16/9
            readonly property real targetH: Math.min(height, width / aspect)
            readonly property real targetW: targetH * aspect
            readonly property real verticalInset: Math.max(0, (height - targetH) / 2)

            VideoView {
                id: video
                anchors.centerIn: parent
                width: videoFrame.targetW
                height: videoFrame.targetH
                source: CameraViewModel.streamUrl
            }
        }

        // Right-side stacked controls: 变焦 / 前后摄 / 录像·拍照 / 云台
        ColumnLayout {
            Layout.preferredWidth: 220
            Layout.minimumWidth: 220
            Layout.maximumWidth: 220
            Layout.fillHeight: true
            Layout.topMargin: videoFrame.verticalInset
            Layout.bottomMargin: videoFrame.verticalInset
            spacing: 8

            // Row 1: 变焦
            GroupBox {
                title: qsTr("变焦")
                Layout.fillWidth: true
                Layout.preferredHeight: 84

                RowLayout {
                    anchors.fill: parent
                    spacing: 6

                    Label { text: qsTr("远") }
                    Slider {
                        id: zoomSlider
                        from: -1; to: 1; value: 0
                        Layout.fillWidth: true
                        onMoved: CameraViewModel.zoom(value)
                        onPressedChanged: if (!pressed) value = 0
                    }
                    Label { text: qsTr("近") }
                }
            }

            // Row 2: 前后摄
            GroupBox {
                title: qsTr("前后摄")
                Layout.fillWidth: true
                Layout.preferredHeight: 144

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    ButtonGroup { id: chGrp }
                    Button {
                        text: qsTr("前摄")
                        checkable: true
                        checked: CameraViewModel.activeChannel === 0
                        ButtonGroup.group: chGrp
                        Layout.fillWidth: true
                        Layout.preferredHeight: panel.controlButtonHeight
                        onClicked: CameraViewModel.switchChannel(0)
                    }
                    Button {
                        text: qsTr("后摄")
                        checkable: true
                        checked: CameraViewModel.activeChannel === 1
                        ButtonGroup.group: chGrp
                        Layout.fillWidth: true
                        Layout.preferredHeight: panel.controlButtonHeight
                        onClicked: CameraViewModel.switchChannel(1)
                    }
                }
            }

            // Row 3: 录像 / 拍照
            GroupBox {
                title: qsTr("录像 / 拍照")
                Layout.fillWidth: true
                Layout.preferredHeight: 144

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Button {
                        text: CameraViewModel.recording ? qsTr("停止录像") : qsTr("开始录像")
                        Layout.fillWidth: true
                        Layout.preferredHeight: panel.controlButtonHeight
                        onClicked: CameraViewModel.recording
                                    ? CameraViewModel.stopRecording()
                                    : CameraViewModel.startRecording()
                    }
                    Button {
                        text: qsTr("拍照")
                        Layout.fillWidth: true
                        Layout.preferredHeight: panel.controlButtonHeight
                        onClicked: CameraViewModel.snapshot()
                    }
                }
            }

            // Row 4: 云台控制 (kept compact)
            GroupBox {
                title: qsTr("云台控制")
                Layout.fillWidth: true
                Layout.preferredHeight: 220
                Layout.fillHeight: true

                GridLayout {
                    anchors.fill: parent
                    columns: 3
                    uniformCellWidths: true
                    uniformCellHeights: true
                    rowSpacing: 4; columnSpacing: 4

                    Button { text: qsTr("远"); Layout.fillWidth: true; Layout.fillHeight: true; onPressed: CameraViewModel.zoom(-1); onReleased: CameraViewModel.zoom(0) }
                    Button { text: "↑"; Layout.fillWidth: true; Layout.fillHeight: true; onPressed: CameraViewModel.pan(0, -1); onReleased: CameraViewModel.pan(0,0) }
                    Button { text: qsTr("近"); Layout.fillWidth: true; Layout.fillHeight: true; onPressed: CameraViewModel.zoom(1); onReleased: CameraViewModel.zoom(0) }

                    Button { text: "←"; Layout.fillWidth: true; Layout.fillHeight: true; onPressed: CameraViewModel.pan(-1, 0); onReleased: CameraViewModel.pan(0,0) }
                    Button { text: qsTr("归中"); Layout.fillWidth: true; Layout.fillHeight: true /* TODO: home gimbal */ }
                    Button { text: "→"; Layout.fillWidth: true; Layout.fillHeight: true; onPressed: CameraViewModel.pan(1, 0); onReleased: CameraViewModel.pan(0,0) }

                    Item {}
                    Button { text: "↓"; Layout.fillWidth: true; Layout.fillHeight: true; onPressed: CameraViewModel.pan(0, 1); onReleased: CameraViewModel.pan(0,0) }
                    Item {}
                }
            }

        }
    }
}
