/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 1.15

Control {
    id: root

    property real angle: 0.0
    property real snapAngle: 15.0
    property real defaultAngle: 0.0
    property bool increaseClockwise: false

    function reset()
    {
        angle = defaultAngle
    }
    
    function increment(up: bool, snap: bool)
    {
        if (up) {
            if (snap) {
                root.angle = Math.floor((root.angle + root.snapAngle) / root.snapAngle) * root.snapAngle;
            } else {
                root.angle += 1;
            }
        } else {
            if (snap) {
                root.angle = Math.ceil((root.angle - root.snapAngle) / root.snapAngle) * root.snapAngle;
            } else {
                root.angle -= 1;
            }
        }
    }

    function adjustOpacity(color, alpha) {
        // In Qt6 we can use Qt.Alpha for this, but not in qt5
        return Qt.rgba(color.r, color.g, color.b, alpha);
    }

    implicitWidth: 40
    implicitHeight: implicitWidth
    focusPolicy: Qt.WheelFocus

    contentItem: Shape {
        id: gaugeShape


        property point center: Qt.point(width / 2.0, height / 2.0)
        property real minSide: Math.min(center.x, center.y)
        property real radius: minSide * 0.9
        property real lineMarkerRadius: minSide - radius
        property real angleInRadians: root.angle * Math.PI / 180.0
        property point anglePoint: Qt.point(center.x + Math.cos(angleInRadians) * radius,
                                            root.increaseClockwise ? center.y + Math.sin(angleInRadians) * radius
                                                                   : center.y - Math.sin(angleInRadians) * radius)
        property bool darkMode: root.palette.window.hslLightness < 0.5
        property color circleColor: darkMode ? root.palette.light : root.palette.dark;
        property color axesColor: root.adjustOpacity(circleColor, 0.75);
        property color bgColor: darkMode ? root.palette.dark: root.palette.light;
        property color angleLineColor: darkMode ? Qt.rgba(1, 1, 1, 255): Qt.rgba(0, 0, 0, 255);
        
        property point topPoint: Qt.point(Math.round(center.x), Math.ceil(center.y - radius))
        property point bottomPoint: Qt.point(Math.round(center.x), Math.floor(center.y + radius))
        property point leftPoint: Qt.point(Math.ceil(center.x - radius), Math.round(center.y))
        property point rightPoint: Qt.point(Math.floor(center.x + radius), Math.round(center.y))

        layer.enabled: true
        layer.smooth: true
        layer.textureSize: Qt.size(width * 2, height * 2)

        // Background
        ShapePath {
            id: backgroundShape
            property real radius: (gaugeShape.bottomPoint.y - gaugeShape.topPoint.y) / 2
            strokeColor: "transparent"
            fillColor: root.enabled
                       ? gaugeShape.bgColor
                       : root.palette.window
            startX: gaugeShape.topPoint.x
            startY: gaugeShape.topPoint.y
            PathArc {
                x: gaugeShape.bottomPoint.x
                y: gaugeShape.bottomPoint.y
                radiusX: backgroundShape.radius
                radiusY: backgroundShape.radius
            }
            PathArc {
                x: gaugeShape.topPoint.x
                y: gaugeShape.topPoint.y
                radiusX: backgroundShape.radius
                radiusY: backgroundShape.radius
            }
        }
        // Axes lines
        ShapePath {
            id: axesLinesShape
            strokeColor: gaugeShape.axesColor
            strokeStyle: ShapePath.DashLine
            capStyle: ShapePath.FlatCap
            dashPattern: [1, 1]
            fillColor: "transparent"
            startX: gaugeShape.topPoint.x
            startY: gaugeShape.topPoint.y + 2
            PathLine {
                x: gaugeShape.bottomPoint.x
                y: gaugeShape.bottomPoint.y - 2
            }
            PathMove {
                x: gaugeShape.leftPoint.x + 2
                y: gaugeShape.leftPoint.y
            }
            PathLine {
                x: gaugeShape.rightPoint.x - 2
                y: gaugeShape.rightPoint.y
            }
        }
        // Outer circle
        ShapePath {
            id: outerCircleShape
            strokeColor: root.activeFocus ? root.palette.highlight : gaugeShape.circleColor
            strokeWidth: root.activeFocus || (root.enabled && gaugeMouseArea.gaugeHovered) ? 2 : 1
            fillColor: "transparent"
            startX: gaugeShape.topPoint.x
            startY: gaugeShape.topPoint.y
            PathArc {
                x: gaugeShape.bottomPoint.x
                y: gaugeShape.bottomPoint.y
                radiusX: backgroundShape.radius
                radiusY: backgroundShape.radius
            }
            PathArc {
                x: gaugeShape.topPoint.x
                y: gaugeShape.topPoint.y
                radiusX: backgroundShape.radius
                radiusY: backgroundShape.radius
            }
        }
        // Angle line
        ShapePath {
            id: angleLineShape
            strokeColor: root.adjustOpacity(gaugeShape.angleLineColor, 0.75);
            fillColor: "transparent"
            startX: gaugeShape.center.x
            startY: gaugeShape.center.y
            PathLine {
                x: gaugeShape.anglePoint.x
                y: gaugeShape.anglePoint.y
            }
        }
        // Angle line markers
        ShapePath {
            id: angleLineMarkersShape
            strokeColor: "transparent"
            fillColor: angleLineShape.strokeColor
            startX: gaugeShape.center.x
            startY: gaugeShape.center.y - gaugeShape.lineMarkerRadius
            PathArc {
                x: gaugeShape.center.x
                y: gaugeShape.center.y + gaugeShape.lineMarkerRadius
                radiusX: gaugeShape.lineMarkerRadius
                radiusY: gaugeShape.lineMarkerRadius
            }
            PathArc {
                x: angleLineMarkersShape.startX
                y: angleLineMarkersShape.startY
                radiusX: gaugeShape.lineMarkerRadius
                radiusY: gaugeShape.lineMarkerRadius
            }
            PathMove {
                x: gaugeShape.anglePoint.x
                y: gaugeShape.anglePoint.y - gaugeShape.lineMarkerRadius
            }
            PathArc {
                x: gaugeShape.anglePoint.x
                y: gaugeShape.anglePoint.y + gaugeShape.lineMarkerRadius
                radiusX: gaugeShape.lineMarkerRadius
                radiusY: gaugeShape.lineMarkerRadius
            }
            PathArc {
                x: gaugeShape.anglePoint.x
                y: gaugeShape.anglePoint.y - gaugeShape.lineMarkerRadius
                radiusX: gaugeShape.lineMarkerRadius
                radiusY: gaugeShape.lineMarkerRadius
            }
        }
    }
    
    MouseArea {
        id: gaugeMouseArea

        property bool gaugeHovered: false
        property real cumulatedTrackpadLength: 0.0

        function setAngleFromPoint(me, pressEvent: bool)
        {
            const radiusSquared = gaugeShape.minSide * gaugeShape.minSide;
            const deltaX = me.x - gaugeShape.center.x;
            const deltaY = me.y - gaugeShape.center.y;
            const distanceSquared = deltaX * deltaX + deltaY * deltaY;
            
            gaugeMouseArea.gaugeHovered = distanceSquared <= radiusSquared;
            if (pressEvent) {
                if (!gaugeMouseArea.gaugeHovered) {
                    return;
                }
                if (root.focusPolicy & Qt.ClickFocus) {
                    root.forceActiveFocus(Qt.MouseFocusReason);
                }
            } else {
                if (!gaugeMouseArea.pressed) {
                    return;
                }
            }

            let a = Math.atan2(root.increaseClockwise ? deltaY : -deltaY, deltaX) * 180 / Math.PI;

            const minimumSnapDistance = 40;
            const snapDistance = Math.max(minimumSnapDistance * minimumSnapDistance, radiusSquared * 4);
            const controlPressed = me.modifiers & Qt.ControlModifier;
            const shiftPressed = me.modifiers & Qt.ShiftModifier;

            if (controlPressed && shiftPressed) {
                a = Math.round(a);
            } else if (!shiftPressed && (controlPressed || distanceSquared < snapDistance)) {
                a = Math.round(a / root.snapAngle) * root.snapAngle;
            }
            
            root.angle = a;
        }

        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true

        onExited: gaugeMouseArea.gaugeHovered = false

        onPressed: (me) => {
            setAngleFromPoint(me, true);
            if (!gaugeMouseArea.gaugeHovered) {
                me.accepted = false;
            }
        }

        onPositionChanged: (me) => {
            setAngleFromPoint(me, false);
        }

        onDoubleClicked: root.reset()

        onWheel: (we) => {
            if (root.focusPolicy & Qt.WheelFocus) {
                root.forceActiveFocus(Qt.MouseFocusReason);
            }

            let inc = 0.0;
            if (we.pixelDelta && we.pixelDelta.y !== 0) {
                cumulatedTrackpadLength += we.pixelDelta.y;
                if (Math.abs(cumulatedTrackpadLength) > 30) {
                    inc = cumulatedTrackpadLength;
                    cumulatedTrackpadLength = 0.0;
                }
            } else {
                inc = we.angleDelta.y;
            }

            if (inc === 0) {
                return;
            }

            inc *= we.inverted ? -1 : 1;
            increment(inc > 0, we.modifiers & Qt.ControlModifier);
        }
    }

    Keys.onPressed: (ke) => {
        if (ke.key !== Qt.Key_Up && ke.key !== Qt.Key_Right &&
            ke.key !== Qt.Key_Down && ke.key !== Qt.Key_Left) {
            return;
        }
        increment(ke.key === Qt.Key_Up || ke.key === Qt.Key_Right,
                  ke.modifiers & Qt.ControlModifier);
    }
}
