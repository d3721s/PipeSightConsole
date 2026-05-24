import QtQuick
import QtQuick.Controls

/**
 * Tiny status pill. Set .state to "connected" / "connecting" / "disconnected".
 */
Item {
    id: root
    implicitWidth: row.implicitWidth + 16
    implicitHeight: 28

    state: "disconnected"

    states: [
        State {
            name: "connected"
            PropertyChanges { target: dot; color: "#4caf50" }
            PropertyChanges { target: lbl; text: qsTr("已连接") }
        },
        State {
            name: "connecting"
            PropertyChanges { target: dot; color: "#ff9800" }
            PropertyChanges { target: lbl; text: qsTr("连接中") }
        },
        State {
            name: "disconnected"
            PropertyChanges { target: dot; color: "#f44336" }
            PropertyChanges { target: lbl; text: qsTr("未连接") }
        }
    ]

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: "#22000000"
        border.color: "#33ffffff"
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        Rectangle { id: dot; width: 10; height: 10; radius: 5; anchors.verticalCenter: parent.verticalCenter }
        Label    { id: lbl; anchors.verticalCenter: parent.verticalCenter }
    }
}
