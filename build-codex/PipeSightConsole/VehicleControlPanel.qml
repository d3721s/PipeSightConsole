import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        GroupBox {
            title: qsTr("运动控制")
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                Joystick {
                    id: stick
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 220
                    Layout.preferredHeight: 220
                    // joystick valueY is +down; throttle is +forward, so negate
                    onMoved: VehicleViewModel.drive(-stick.valueY, stick.valueX)
                    onReleased: VehicleViewModel.drive(0, 0)
                }

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: qsTr("目标速度 (m/s)") }
                    Slider {
                        id: speedSlider
                        from: 0; to: 1; value: 0.3
                        stepSize: 0.05
                        Layout.fillWidth: true
                        onMoved: VehicleViewModel.setTargetSpeed(value)
                    }
                    Label { text: speedSlider.value.toFixed(2) }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Button { text: qsTr("急停"); Layout.fillWidth: true; onClicked: VehicleViewModel.stop() }
                    Button { text: qsTr("位置归零"); Layout.fillWidth: true; onClicked: VehicleViewModel.resetHome() }
                }
            }
        }

        GroupBox {
            title: qsTr("灯光")
            Layout.fillWidth: true
            GridLayout {
                columns: 3; rowSpacing: 8
                Label { text: qsTr("前灯") }
                Slider {
                    id: frontL
                    from: 0; to: 100; value: VehicleViewModel.frontLight
                    Layout.fillWidth: true
                    onMoved: VehicleViewModel.frontLight = value
                }
                Label { text: Math.round(frontL.value) + "%" }

                Label { text: qsTr("后灯") }
                Slider {
                    id: rearL
                    from: 0; to: 100; value: VehicleViewModel.rearLight
                    Layout.fillWidth: true
                    onMoved: VehicleViewModel.rearLight = value
                }
                Label { text: Math.round(rearL.value) + "%" }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
