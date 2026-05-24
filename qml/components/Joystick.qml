import QtQuick

/**
 * Touch-first 2-axis joystick. Emits .x and .y in [-1, 1].
 *
 * Signals:
 *   moved    — fired continuously while dragging
 *   released — fired once on release; consumers usually send drive(0,0)
 */
Item {
    id: root
    width: 200
    height: 200

    property real valueX: 0   // [-1, 1]; positive = right
    property real valueY: 0   // [-1, 1]; positive = down (forward = -1 in screen coords)

    signal moved()
    signal released()

    Rectangle {
        id: base
        anchors.fill: parent
        radius: width / 2
        color: "#1e2630"
        border.color: "#3a4654"
        border.width: 2
    }

    Rectangle {
        id: knob
        width: 60; height: 60; radius: 30
        color: "#4fc3f7"
        x: base.width/2 - width/2
        y: base.height/2 - height/2

        MouseArea {
            anchors.fill: parent
            drag.target: knob
            drag.axis: Drag.XAndYAxis
            drag.minimumX: 0
            drag.maximumX: base.width  - knob.width
            drag.minimumY: 0
            drag.maximumY: base.height - knob.height

            onPositionChanged: {
                const cx = base.width/2  - knob.width/2
                const cy = base.height/2 - knob.height/2
                root.valueX = (knob.x - cx) / cx
                root.valueY = (knob.y - cy) / cy
                root.moved()
            }
            onReleased: {
                returnAnim.start()
                root.released()
            }
        }
    }

    ParallelAnimation {
        id: returnAnim
        NumberAnimation { target: knob; property: "x"; to: base.width/2  - knob.width/2;  duration: 120 }
        NumberAnimation { target: knob; property: "y"; to: base.height/2 - knob.height/2; duration: 120 }
        onStopped: { root.valueX = 0; root.valueY = 0 }
    }
}
