import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

Item {
    id: panel
    readonly property int fieldHeight: 44
    readonly property int cameraFieldWidth: 150
    readonly property int compactFieldWidth: 110
    readonly property int buttonWidth: 86

    ScrollView {
        anchors.fill: parent
        contentWidth: width

        ColumnLayout {
            width: parent.width
            spacing: 10

            // ---- Camera 1 (前摄) ----
            CameraConfigRow {
                Layout.fillWidth: true
                channel: 0
                title: qsTr("前置摄像头")
                fieldHeight: panel.fieldHeight
                compactFieldWidth: panel.compactFieldWidth
                buttonWidth: panel.buttonWidth
            }

            // ---- Camera 2 (后摄) ----
            CameraConfigRow {
                Layout.fillWidth: true
                channel: 1
                title: qsTr("后置摄像头")
                fieldHeight: panel.fieldHeight
                compactFieldWidth: panel.compactFieldWidth
                buttonWidth: panel.buttonWidth
            }

            // ---- Stereo ----
            GroupBox {
                title: qsTr("双目相机参数")
                Layout.fillWidth: true
                RowLayout {
                    width: parent.width
                    spacing: 10

                    Label { text: qsTr("曝光") }
                    Slider { id: exp; from: 0; to: 100; value: 50; Layout.preferredWidth: 320
                             onMoved: ConfigViewModel.setStereoParam("exposure", value) }
                    Label { text: qsTr("白平衡") }
                    ComboBox {
                        id: wbBox
                        Layout.preferredWidth: 180
                        Layout.preferredHeight: panel.fieldHeight
                        model: ["Auto", "Daylight", "Cloudy", "Tungsten"]
                        onActivated: ConfigViewModel.setStereoParam("whiteBalance", currentText)
                    }
                    CheckBox {
                        id: syncExposure
                        text: qsTr("同步曝光")
                        onToggled: ConfigViewModel.setStereoParam("sync", checked)
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: qsTr("应用")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: {
                            ConfigViewModel.setStereoParam("exposure", exp.value)
                            ConfigViewModel.setStereoParam("whiteBalance", wbBox.currentText)
                            ConfigViewModel.setStereoParam("sync", syncExposure.checked)
                        }
                    }
                }
            }

            // ---- Radar ----
            GroupBox {
                title: qsTr("激光雷达参数")
                Layout.fillWidth: true
                RowLayout {
                    width: parent.width
                    spacing: 10

                    Label { text: qsTr("扫描频率 (Hz)") }
                    SpinBox { id: scanHzBox; from: 1; to: 50; value: 10; Layout.preferredWidth: panel.compactFieldWidth; Layout.preferredHeight: panel.fieldHeight
                              onValueModified: ConfigViewModel.setRadarParam("scanHz", value) }
                    Label { text: qsTr("扫描角度 (°)") }
                    SpinBox { id: angleDegBox; from: 30; to: 360; value: 270; Layout.preferredWidth: panel.compactFieldWidth; Layout.preferredHeight: panel.fieldHeight
                              onValueModified: ConfigViewModel.setRadarParam("angleDeg", value) }
                    Label { text: qsTr("距离范围 (m)") }
                    SpinBox { id: rangeMBox; from: 1; to: 100; value: 30; Layout.preferredWidth: panel.compactFieldWidth; Layout.preferredHeight: panel.fieldHeight
                              onValueModified: ConfigViewModel.setRadarParam("rangeM", value) }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: qsTr("应用")
                        Layout.preferredWidth: panel.buttonWidth
                        Layout.preferredHeight: panel.fieldHeight
                        onClicked: {
                            ConfigViewModel.setRadarParam("scanHz", scanHzBox.value)
                            ConfigViewModel.setRadarParam("angleDeg", angleDegBox.value)
                            ConfigViewModel.setRadarParam("rangeM", rangeMBox.value)
                        }
                    }
                }
            }

            // ---- Recording ----
            GroupBox {
                title: qsTr("录像配置")
                Layout.fillWidth: true
                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
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
                        Label { text: qsTr("循环录像") }
                        Switch {
                            id: cyclicRecordSwitch
                            checked: ConfigViewModel.cyclicRecord
                            onToggled: ConfigViewModel.cyclicRecord = checked
                        }
                        Item { Layout.fillWidth: true }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Label { text: qsTr("存储路径") }
                        TextField {
                            id: storagePathField
                            Layout.fillWidth: true
                            Layout.preferredHeight: panel.fieldHeight
                            text: ConfigViewModel.storagePath
                            onEditingFinished: ConfigViewModel.storagePath = text
                        }
                        Button {
                            text: qsTr("应用")
                            Layout.preferredWidth: panel.buttonWidth
                            Layout.preferredHeight: panel.fieldHeight
                            onClicked: {
                                ConfigViewModel.segmentMinutes = segmentMinutesBox.value
                                ConfigViewModel.cyclicRecord = cyclicRecordSwitch.checked
                                ConfigViewModel.storagePath = storagePathField.text
                            }
                        }
                    }
                }
            }

            OsdPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 136
            }
        }
    }
}
