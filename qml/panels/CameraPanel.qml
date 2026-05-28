import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import PipeSightConsole

Item {
    id: panel
    readonly property int controlButtonHeight: 48
    readonly property int controlPanelWidth: 230
    readonly property int controlSpacing: 8

    component PtzButton: Button {
        id: ptzButton
        property int buttonSize: 58

        width: buttonSize
        height: buttonSize
        padding: 0
        font.pixelSize: 24

        background: Rectangle {
            radius: width / 2
            color: ptzButton.down ? "#53545a" : (ptzButton.hovered ? "#48494f" : "#3f4045")
            border.color: "#5b5b64"
            border.width: 1
        }

        contentItem: Text {
            text: ptzButton.text
            color: "#e9edf3"
            font: ptzButton.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Item {
        anchors.fill: parent
        anchors.margins: 4

        Item {
            id: videoFrame
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: panel.controlPanelWidth + 6

            VideoView {
                id: video
                anchors.fill: parent
                source: CameraViewModel.streamUrl
            }
        }

        // Right-side panel is fixed horizontally; its contents may scroll vertically.
        Flickable {
            id: controlsPane
            width: panel.controlPanelWidth
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
            contentWidth: width
            contentHeight: scrollContent.implicitHeight
            interactive: contentHeight > height
            boundsBehavior: interactive ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AlwaysOff }
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            Item {
                id: scrollContent
                width: controlsPane.width
                height: controlsLayout.implicitHeight
                implicitWidth: controlsPane.width
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

                        RowLayout {
                            anchors.fill: parent
                            spacing: 6

                            Label {
                                text: qsTr("远")
                                Layout.alignment: Qt.AlignVCenter
                            }
                            Slider {
                                id: zoomSlider
                                from: -1; to: 1; value: 0
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                onMoved: CameraViewModel.zoom(value)
                                onPressedChanged: if (!pressed) value = 0
                            }
                            Label {
                                text: qsTr("近")
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }
                    }

                    // Row 2: 摄像头切换
                    GroupBox {
                        title: qsTr("摄像头切换")
                        Layout.fillWidth: true

                        RowLayout {
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

                    // Row 3: 操作
                    GroupBox {
                        title: qsTr("操作")
                        Layout.fillWidth: true

                        RowLayout {
                            anchors.fill: parent
                            spacing: 6

                            Button {
                                text: CameraViewModel.recording ? qsTr("停止录像") : qsTr("开始录像")
                                Layout.fillWidth: true
                                Layout.preferredHeight: panel.controlButtonHeight
                                Material.foreground: CameraViewModel.recording ? "#e53935" : undefined
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
                        Layout.preferredHeight: 360
                        Layout.minimumHeight: 336

                        Item {
                            id: ptzPad
                            readonly property int buttonSize: 58
                            readonly property int gapX: 13
                            readonly property int gapY: 8

                            width: buttonSize * 3 + gapX * 2
                            height: buttonSize * 5 + gapY * 4
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter

                            PtzButton {
                                text: "^"
                                buttonSize: ptzPad.buttonSize
                                x: ptzPad.buttonSize + ptzPad.gapX
                                y: 0
                                onPressed: CameraViewModel.pan(0, -1)
                                onReleased: CameraViewModel.pan(0, 0)
                            }

                            PtzButton {
                                text: "<"
                                buttonSize: ptzPad.buttonSize
                                x: 0
                                y: ptzPad.buttonSize + ptzPad.gapY
                                onPressed: CameraViewModel.pan(-1, 0)
                                onReleased: CameraViewModel.pan(0, 0)
                            }
                            PtzButton {
                                text: ">"
                                buttonSize: ptzPad.buttonSize
                                x: (ptzPad.buttonSize + ptzPad.gapX) * 2
                                y: ptzPad.buttonSize + ptzPad.gapY
                                onPressed: CameraViewModel.pan(1, 0)
                                onReleased: CameraViewModel.pan(0, 0)
                            }

                            PtzButton {
                                text: "v"
                                buttonSize: ptzPad.buttonSize
                                x: ptzPad.buttonSize + ptzPad.gapX
                                y: (ptzPad.buttonSize + ptzPad.gapY) * 2
                                onPressed: CameraViewModel.pan(0, 1)
                                onReleased: CameraViewModel.pan(0, 0)
                            }

                            PtzButton {
                                text: "+"
                                buttonSize: ptzPad.buttonSize
                                x: (ptzPad.buttonSize + ptzPad.gapX) * 2
                                y: (ptzPad.buttonSize + ptzPad.gapY) * 3
                                onPressed: CameraViewModel.zoom(1)
                                onReleased: CameraViewModel.zoom(0)
                            }

                            PtzButton {
                                text: "-"
                                buttonSize: ptzPad.buttonSize
                                x: (ptzPad.buttonSize + ptzPad.gapX) * 2
                                y: (ptzPad.buttonSize + ptzPad.gapY) * 4
                                onPressed: CameraViewModel.zoom(-1)
                                onReleased: CameraViewModel.zoom(0)
                            }
                        }
                    }
                }
            }
        }
    }
}
