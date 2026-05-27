import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 8

        // Center: stereo / point-cloud display placeholder.
        // Hook a custom QQuickItem (e.g. point-cloud renderer) here later.
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#202830"
            border.color: "#444"
            Label {
                anchors.centerIn: parent
                color: "#888"
                text: LaserViewModel.pointCloudEnabled
                      ? qsTr("点云 / 深度图渲染区(占位)")
                      : qsTr("双目实时画面(占位)")
            }
        }

        // Bottom row: actions
        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("标记数: %1").arg(LaserViewModel.markerCount) }
            Item { Layout.fillWidth: true }
            Button { text: qsTr("拍照");           onClicked: LaserViewModel.captureSnapshot() }
            Button {
                text: LaserViewModel.pointCloudEnabled ? qsTr("关闭三维显示") : qsTr("显示三维图片")
                onClicked: LaserViewModel.pointCloudEnabled = !LaserViewModel.pointCloudEnabled
            }
            Button {
                text: qsTr("添加标记")
                onClicked: markerDialog.open()
            }
        }
    }

    Dialog {
        id: markerDialog
        title: qsTr("添加标记")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        ColumnLayout {
            ComboBox {
                id: catBox
                model: ["裂缝", "变形", "接口", "其他"]
                Layout.fillWidth: true
            }
            TextField {
                id: noteField
                placeholderText: qsTr("备注")
                Layout.fillWidth: true
            }
        }
        onAccepted: LaserViewModel.addMarker(catBox.currentText, noteField.text)
    }
}
