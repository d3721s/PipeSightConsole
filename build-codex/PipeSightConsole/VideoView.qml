import QtQuick
import QtQuick.Controls
import QtMultimedia

/**
 * Wraps a QtMultimedia VideoOutput. Bind .source to an RTSP/HTTP URL.
 *
 * If RTSP latency or compatibility is poor on Android, swap the implementation
 * here to a custom QQuickItem fed by an FFmpeg pipeline — callers won't change.
 */
Item {
    id: root
    property url source

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    MediaPlayer {
        id: player
        source: root.source
        videoOutput: vo
        audioOutput: AudioOutput { muted: true }
        onSourceChanged: if (source.toString().length) player.play()
    }

    VideoOutput {
        id: vo
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
    }

    Label {
        anchors.centerIn: parent
        visible: !root.source || root.source.toString().length === 0
        color: "#888"
        text: qsTr("无视频源 — 请在「配置」中设置摄像头 IP 与码流")
    }
}
