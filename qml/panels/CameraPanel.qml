import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

Item {
    id: panel
    readonly property int controlButtonHeight: 48
    readonly property int controlPanelWidth: 230
    readonly property int controlSpacing: 8

    Item {
        anchors.fill: parent
        anchors.margins: 4

        Item {
            id: videoFrame
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: controlsPane.left
            anchors.rightMargin: 6

            VideoView {
                id: video
                anchors.fill: parent
                source: CameraViewModel.streamUrl
            }
        }

        // Right-side panel is fixed horizontally; its contents may scroll vertically.
        ScrollView {
            id: controlsPane
            width: panel.controlPanelWidth
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
            contentWidth: availableWidth
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Item {
                width: controlsPane.availableWidth
                height: controlsLayout.implicitHeight
                implicitWidth: controlsPane.availableWidth
                implicitHeight: controlsLayout.implicitHeight

                ColumnLayout {
                    id: controlsLayout
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    spacing: panel.controlSpacing

                    // Row 1: 变焦
                    GroupBox {
                        title: qsTr("变焦")
                        Layout.fillWidth: true
                        Layout.preferredHeight: 76

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
                        Layout.preferredHeight: 132

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
                        Layout.preferredHeight: 132

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

                    // Row 4: 云台控制
                    GroupBox {
                        title: qsTr("云台控制")
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        Layout.minimumHeight: 156

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
    }
}
