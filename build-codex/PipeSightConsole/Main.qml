import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    minimumWidth: 1280
    maximumWidth: 1280
    minimumHeight: 800
    maximumHeight: 800
    visible: true
    title: qsTr("PipeSight Console")

    Material.theme: Material.Dark
    Material.accent: Material.Cyan

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            Label {
                text: qsTr("PipeSight 管道检测控制台")
                font.pixelSize: 18
                Layout.alignment: Qt.AlignVCenter
            }
            Item { Layout.fillWidth: true }
            // Placeholder connection indicator. Wire to TcpClient state via VM later.
            ConnectionIndicator { state: "connected" }
        }
    }

    TabBar {
        id: tabs
        width: parent.width
        TabButton { text: qsTr("摄像头") }
        TabButton { text: qsTr("激光/双目") }
        TabButton { text: qsTr("小车") }
        TabButton { text: qsTr("配置") }
        TabButton { text: qsTr("高级") }
    }

    StackLayout {
        anchors.top: tabs.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: tabs.currentIndex

        CameraPanel {}
        LaserPanel {}
        // Vehicle tab combines info + control side by side
        SplitView {
            orientation: Qt.Horizontal
            VehicleInfoPanel {
                SplitView.preferredWidth: 360
                SplitView.minimumWidth: 360
                SplitView.maximumWidth: 360
            }
            VehicleControlPanel { SplitView.fillWidth: true }
        }
        ConfigPanel {}
        AdvancedPanel {}
    }
}
