import QtQuick
import QtQuick.Controls
import PipeSightConsole

Popup {
    id: snackbar

    property bool isError: false
    property int duration: 3200
    property int bottomMargin: Qt.platform.os === "android" ? 32 : 24
    readonly property int maxSnackbarWidth: parent ? Math.max(240, parent.width - 32) : 320

    function show(message, error) {
        snackbar.isError = error === true
        snackbarText.text = String(message)
        snackbar.open()
        closeTimer.restart()
    }

    parent: Overlay.overlay
    modal: false
    focus: false
    closePolicy: Popup.NoAutoClose
    leftPadding: 24
    rightPadding: 24
    topPadding: 14
    bottomPadding: 14
    width: Math.min(maxSnackbarWidth,
                    Math.max(320, snackbarText.implicitWidth + leftPadding + rightPadding))
    height: Math.max(48, snackbarText.implicitHeight + topPadding + bottomPadding)
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? parent.height - height - bottomMargin : 0

    Connections {
        target: AppNotifier
        function onNotificationRequested(message, isError) {
            snackbar.show(message, isError)
        }
    }

    Timer {
        id: closeTimer
        interval: snackbar.duration
        onTriggered: snackbar.close()
    }

    background: Rectangle {
        color: snackbar.isError ? "#4a1f1f" : "#323232"
        radius: 4
        border.color: snackbar.isError ? "#ef5350" : "transparent"
        border.width: snackbar.isError ? 1 : 0
    }

    contentItem: Label {
        id: snackbarText
        width: snackbar.width - snackbar.leftPadding - snackbar.rightPadding
        color: "white"
        font.pixelSize: 14
        wrapMode: Text.Wrap
        maximumLineCount: 3
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 140 }
    }

    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 120 }
    }
}
