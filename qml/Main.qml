import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import PipeSightConsole

ApplicationWindow {
    id: root
    readonly property bool isAndroid: Qt.platform.os === "android"
    property bool backExitArmed: false

    width: 1280
    height: 800
    minimumWidth: 1280
    minimumHeight: 800
    maximumWidth: 1280
    maximumHeight: 800
    visible: true
    title: qsTr("PipeSight Console")

    Material.theme: Material.Dark
    Material.accent: Material.Cyan

    onClosing: function(close) {
        if (!root.isAndroid)
            return

        if (root.backExitArmed) {
            backExitResetTimer.stop()
            return
        }

        close.accepted = false
        root.backExitArmed = true
        AppNotifier.info(qsTr("再按一次返回退出"))
        backExitResetTimer.restart()
    }

    Timer {
        id: backExitResetTimer
        interval: 2000
        repeat: false
        onTriggered: root.backExitArmed = false
    }

    Connections {
        target: CameraViewModel
        function onRecordingNotification(message, isError) {
            AppNotifier.show(message, isError)
        }
    }

    Connections {
        target: ConfigViewModel
        function onConfigChanged() {
            VehicleViewModel.refreshMs = ConfigViewModel.vehicleInfoRefreshMs
        }
    }

    Connections {
        target: NativeNotifier
        function onNotificationFailed(message) {
            AppNotifier.info(message)
        }
    }

    AppSnackbar {}

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            Label {
                text: qsTr("PipeSight 管道检测控制台")
                font.pixelSize: 18
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }
            // Placeholder connection indicators. Wire to device states via VMs later.
            RowLayout {
                spacing: 8
                ConnectionIndicator { deviceName: qsTr("前摄像头") }
                ConnectionIndicator { deviceName: qsTr("后摄像头") }
                ConnectionIndicator { deviceName: qsTr("深度相机") }
                ConnectionIndicator { deviceName: qsTr("移动底盘") }
            }
        }
    }

    TabBar {
        id: tabs
        width: parent.width
        TabButton { text: qsTr("摄像头") }
        TabButton { text: qsTr("深度相机") }
        TabButton { text: qsTr("移动底盘") }
        TabButton { text: qsTr("高级") }
        TabButton { text: qsTr("配置") }
    }

    StackLayout {
        anchors.top: tabs.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: tabs.currentIndex

        CameraPanel {}
        DepthCameraPanel {}
        // Vehicle tab keeps left/right panes fixed; each pane scrolls vertically.
        Item {
            VehicleInfoPanel {
                id: vehicleInfo
                width: root.isAndroid ? 330 : 360
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }
            VehicleControlPanel {
                anchors.left: vehicleInfo.right
                anchors.leftMargin: 8
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }
        }
        AdvancedPanel {}
        ConfigPanel {}
    }

}
