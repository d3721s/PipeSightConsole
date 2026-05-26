import QtQuick
import QtQuick.Controls
import QtMultimedia

/**
 * Wraps a QtMultimedia VideoOutput. Bind .source to an RTSP/HTTP URL.
 *
 * Requires the FFmpeg backend (QT_MEDIA_BACKEND=ffmpeg, set in main.cpp) — the
 * default Windows WMF backend can't demux rtsp://.
 */
Item {
    id: root
    property url source
    property string lastError: ""

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    MediaPlayer {
        id: player
        source: root.source
        videoOutput: vo
        audioOutput: AudioOutput { }
        onSourceChanged: {
            root.lastError = ""
            if (source.toString().length) player.play()
        }
        onErrorOccurred: (err, str) => root.lastError = str
    }

    VideoOutput {
        id: vo
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
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
            if (player.mediaStatus === MediaPlayer.LoadingMedia
                    || player.mediaStatus === MediaPlayer.BufferingMedia)
                return qsTr("加载中…")
            return ""
        }
    }
}
