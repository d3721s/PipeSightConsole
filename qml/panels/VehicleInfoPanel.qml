import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

Item {
    ScrollView {
        id: scroller
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        Item {
            width: scroller.availableWidth
            height: contentLayout.implicitHeight + 24
            implicitWidth: scroller.availableWidth
            implicitHeight: contentLayout.implicitHeight + 24

            ColumnLayout {
                id: contentLayout
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 4
                spacing: 12

                GroupBox {
                    title: qsTr("姿态")
                    Layout.fillWidth: true
                    GridLayout {
                        columns: 2
                        rowSpacing: 4
                        columnSpacing: 16
                        Label { text: qsTr("俯仰角 (Pitch)") }
                        Label { text: VehicleViewModel.pitchDeg.toFixed(1) + "°" }
                        Label { text: qsTr("横滚角 (Roll)") }
                        Label { text: VehicleViewModel.rollDeg.toFixed(1)  + "°" }
                        Label { text: qsTr("航向角 (Yaw)") }
                        Label { text: VehicleViewModel.yawDeg.toFixed(1)   + "°" }
                    }
                }

                GroupBox {
                    title: qsTr("速度 / 里程")
                    Layout.fillWidth: true
                    GridLayout {
                        columns: 2
                        rowSpacing: 4
                        columnSpacing: 16
                        Label { text: qsTr("当前速度") }
                        Label { text: (VehicleViewModel.speedMps).toFixed(2) + " m/s" }
                        Label { text: qsTr("累计里程") }
                        Label { text: VehicleViewModel.odometerM.toFixed(2) + " m" }
                    }
                }

                GroupBox {
                    title: qsTr("电量 / 信号")
                    Layout.fillWidth: true
                    GridLayout {
                        columns: 2
                        rowSpacing: 4
                        columnSpacing: 16
                        Label { text: qsTr("电池") }
                        ProgressBar {
                            from: 0
                            to: 100
                            value: VehicleViewModel.batteryPct
                            Layout.fillWidth: true
                        }
                        Label { text: qsTr("电压") }
                        Label { text: VehicleViewModel.batteryV.toFixed(2) + " V" }
                        Label { text: qsTr("信号质量") }
                        ProgressBar {
                            from: 0
                            to: 100
                            value: VehicleViewModel.signalPct
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
