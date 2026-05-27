import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog

    property string message: qsTr("确认应用当前设置？")
    property var acceptedCallback: null

    function confirm(messageText, callback) {
        message = messageText
        acceptedCallback = callback
        open()
    }

    parent: Overlay.overlay
    modal: true
    focus: true
    dim: true
    title: qsTr("确认操作")
    standardButtons: Dialog.Ok | Dialog.Cancel
    closePolicy: Popup.CloseOnEscape
    width: 360
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    onAccepted: {
        const callback = acceptedCallback
        acceptedCallback = null
        if (callback)
            callback()
    }
    onRejected: acceptedCallback = null
    onClosed: if (!visible) acceptedCallback = null

    ColumnLayout {
        width: parent.width

        Label {
            text: dialog.message
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }
}
