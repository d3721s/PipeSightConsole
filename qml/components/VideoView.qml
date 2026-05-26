import QtQuick
import QtQuick.Controls
import QtMultimedia

Item {
    id: root
    property url source
    property string lastError: ""

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    VideoOutput {
        id: vo
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
    }

    MediaPlayer {
        id: player
        videoOutput: vo
        audioOutput: AudioOutput { }
        onErrorOccurred: (err, str) => {
            root.lastError = str
            reconnectTimer.restart()
        }
        onMediaStatusChanged: {
            if (mediaStatus === MediaPlayer.EndOfMedia
                    || mediaStatus === MediaPlayer.InvalidMedia)
                reconnectTimer.restart()
        }
    }

    Timer {
        id: reconnectTimer
        interval: 1500
        repeat: false
        onTriggered: play(root.source.toString())
    }

    Label {
        anchors.centerIn: parent
        width: parent.width - 32
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        color: root.lastError.length ? "#ff8080" : "#888"
        visible: text.length > 0
        text: {
            if (root.lastError.length)
                return qsTr("播放失败：") + root.lastError
            if (!root.source || root.source.toString().length === 0)
                return qsTr("无视频源 — 请在「配置」中设置摄像头 IP 与码流")
            if (player.playbackState !== MediaPlayer.PlayingState
                    && player.mediaStatus === MediaPlayer.LoadingMedia)
                return qsTr("加载中…")
            return ""
        }
    }

    function play(url) {
        root.lastError = ""
        player.source = ""
        if (url && url.length) {
            player.source = url
            player.play()
        }
    }

    onSourceChanged: play(root.source.toString())
    Component.onCompleted: play(root.source.toString())
}
