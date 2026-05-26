import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PipeSightConsole

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#202830"
            border.color: "#444"
            Label {
                anchors.centerIn: parent
                color: "#888"
                text: DepthCameraViewModel.depthCloudEnabled
                      ? qsTr("深度点云渲染区(占位)")
                      : qsTr("深度相机实时画面(占位)")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("标记数: %1").arg(DepthCameraViewModel.depthMarkerCount) }
            Item { Layout.fillWidth: true }
            Button { text: qsTr("拍照"); onClicked: DepthCameraViewModel.captureDepthSnapshot() }
            Button {
                text: DepthCameraViewModel.depthCloudEnabled ? qsTr("关闭三维显示") : qsTr("显示三维图片")
                onClicked: DepthCameraViewModel.depthCloudEnabled = !DepthCameraViewModel.depthCloudEnabled
            }
            Button {
                text: qsTr("添加标记")
                onClicked: depthMarkerDialog.open()
            }
        }
    }

    Dialog {
        id: depthMarkerDialog
        title: qsTr("添加标记")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        ColumnLayout {
            ComboBox {
                id: depthCatBox
                model: ["裂缝", "变形", "接口", "其他"]
                Layout.fillWidth: true
            }
            TextField {
                id: depthNoteField
                placeholderText: qsTr("备注")
                Layout.fillWidth: true
            }
        }
        onAccepted: DepthCameraViewModel.addDepthMarker(depthCatBox.currentText, depthNoteField.text)
    }
}
