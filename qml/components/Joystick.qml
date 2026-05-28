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
    readonly property real baseSize: Math.min(width, height)

    signal moved()
    signal released()

    function updateKnob(pointerX, pointerY) {
        const center = base.width / 2
        const maxDistance = base.width / 2
        var dx = pointerX - center
        var dy = pointerY - center
        const distance = Math.sqrt(dx * dx + dy * dy)

        if (distance > maxDistance && distance > 0) {
            dx = dx / distance * maxDistance
            dy = dy / distance * maxDistance
        }

        knob.x = center + dx - knob.width / 2
        knob.y = center + dy - knob.height / 2
        root.valueX = maxDistance > 0 ? dx / maxDistance : 0
        root.valueY = maxDistance > 0 ? dy / maxDistance : 0
        root.moved()
    }

    Rectangle {
        id: base
        width: root.baseSize
        height: root.baseSize
        anchors.centerIn: parent
        radius: width / 2
        color: "#1e2630"
        border.color: "#3a4654"
        border.width: 2

        Rectangle {
            id: knob
            width: base.width * 0.3
            height: width
            radius: width / 2
            color: "#4fc3f7"
            x: base.width/2 - width/2
            y: base.height/2 - height/2
        }

        MouseArea {
            anchors.fill: parent
            preventStealing: true

            onPressed: function(mouse) {
                returnAnim.stop()
                root.updateKnob(mouse.x, mouse.y)
            }
            onPositionChanged: function(mouse) {
                if (pressed)
                    root.updateKnob(mouse.x, mouse.y)
            }
            onReleased: {
                returnAnim.start()
                root.released()
            }
            onCanceled: {
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
