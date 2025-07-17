/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Shapes 2.15
import org.krita.components 1.0

Control {
    id: root

    property KisCubicCurve curve: KisCubicCurve {}
    property int selectedKnotIndex: 0
    property bool readOnly: false

    padding: 1
    focusPolicy: Qt.StrongFocus

    onPaletteChanged: contentItem.requestPaint()
    onActiveFocusChanged: contentItem.requestPaint()

    background: Rectangle {
        id: backgroundItem

        readonly property real topPadding: root.topPadding ? root.topPadding : root.padding
        readonly property real bottomPadding: root.bottomPadding ? root.bottomPadding : root.padding
        readonly property real leftPadding: root.leftPadding ? root.leftPadding : root.padding
        readonly property real rightPadding: root.rightPadding ? root.rightPadding : root.padding
        readonly property real lineWidth: root.width - leftPadding - rightPadding
        readonly property real lineHeight: root.height - topPadding - bottomPadding

        color: root.palette.window
        border.color: root.activeFocus ? root.palette.highlight : root.palette.text

        Rectangle {
            anchors.fill: parent
            anchors.topMargin: backgroundItem.topPadding
            anchors.leftMargin: backgroundItem.leftPadding
            anchors.bottomMargin: backgroundItem.bottomPadding
            anchors.rightMargin: backgroundItem.rightPadding
            color: root.palette.base
        }

        Repeater {
            model: 3
            delegate: Rectangle {
                color: root.palette.window;
                width: 1
                height: backgroundItem.lineHeight
                x: backgroundItem.lineWidth * (index + 1) / 4 + backgroundItem.leftPadding
                y: backgroundItem.topPadding
            }
        }

        Repeater {
            model: 3
            delegate: Rectangle {
                color: root.palette.window;
                width: backgroundItem.lineWidth
                height: 1
                x: backgroundItem.leftPadding
                y: backgroundItem.lineHeight * (index + 1) / 4 + backgroundItem.topPadding
            }
        }
    }

    contentItem: Canvas {
        id: contentItem

        property list<point> points

        function updatePoints(pts) {
            const width = contentItem.width;
            const height = contentItem.height;
            const pointYs = root.curve.floatTransfer(width);
            if (pointYs.length === 0) {
                return [];
            }
            let points = [Qt.point(0.5, (height - 1) - pointYs[0] * (height - 1) + 0.5)];
            let lastAngle = 0;
            for (let i = 1; i < pointYs.length; i++) {
                let x = i + 0.5;
                let y = (height - 1) - pointYs[i] * (height - 1) + 0.5;
                if (i === 1) {
                    points.push(Qt.point(x, y));
                } else {
                    const angle = Math.atan(y - points[points.length - 1].y);
                    if (Math.abs(angle - lastAngle) > 0.01) {
                        points.push(Qt.point(x, y));
                    } else {
                        points[points.length - 1] = Qt.point(x, y);
                    }
                }
                lastAngle = Math.atan2(y - points[points.length - 2].y, x - points[points.length - 2].x);
            }
            contentItem.points = points;
        }

        implicitWidth: 256
        implicitHeight: 256

        onPointsChanged: contentItem.requestPaint()

        onPaint: {
            if (!contentItem.points || contentItem.points.length === 0) {
                return;
            }
            var ctx = getContext("2d");

            ctx.clearRect(0, 0, contentItem.width, contentItem.height);
            // Paint the curve
            ctx.fillStyle = Qt.alpha(root.palette.text, 0.2);
            ctx.strokeStyle = root.palette.text;
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(0, 0);
            ctx.moveTo(contentItem.width + 10, contentItem.height + 10);
            ctx.lineTo(-10, contentItem.height + 10);
            ctx.lineTo(-10, contentItem.points[0].y);
            for (let i = 0; i < contentItem.points.length; ++i) {
                ctx.lineTo(contentItem.points[i].x, contentItem.points[i].y);
            }
            ctx.lineTo(contentItem.width + 10, contentItem.points[contentItem.points.length - 1].y);
            ctx.closePath();
            ctx.fill();
            ctx.stroke();
            // Paint non selected knots
            ctx.beginPath();
            ctx.fillStyle = Qt.alpha(root.palette.base, 0.3);
            for (let i = 0; i < root.curve.points.length; ++i) {
                if (i === root.selectedKnotIndex && root.activeFocus) {
                    continue;
                }
                const p = root.curve.points[i];
                ctx.ellipse(p.x * contentItem.width - 6, contentItem.height - p.y * contentItem.height - 6, 12, 12);
            }
            ctx.fill();
            ctx.stroke();
            // Paint the selected knot
            if (root.activeFocus) {
                ctx.beginPath();
                ctx.fillStyle = Qt.alpha(root.palette.highlight, 0.8);
                ctx.strokeStyle = Qt.tint(root.palette.text, Qt.alpha(root.palette.highlight, 0.2));
                const p = root.curve.points[root.selectedKnotIndex];
                ctx.ellipse(p.x * contentItem.width - 6, contentItem.height - p.y * contentItem.height - 6, 12, 12);
                ctx.fill();
                ctx.stroke();
            }
        }

        Component.onCompleted: contentItem.updatePoints()

        MouseArea {
            id: mouseArea

            property bool isDragging: false
            property int draggedAwayKnotIndex: -1
            property point draggedAwayKnot
            property point grabbedKnotOriginalPosition
            property point grabbedKnotOffset

            anchors.fill: parent
            hoverEnabled: true
            preventStealing: true

            function closestKnotIndex(x: real, y: real) : int
            {
                let closestKnot = -1;
                let nearestDistanceSquared = 144;

                for (let i = 0; i < root.curve.points.length; ++i) {
                    const delta = Qt.point(x - root.curve.points[i].x * (mouseArea.width - 1),
                                           (mouseArea.height - 1 - y) - root.curve.points[i].y * (mouseArea.height - 1));
                    const distanceSquared = delta.x * delta.x + delta.y * delta.y;
                    if (distanceSquared < nearestDistanceSquared) {
                        closestKnot = i;
                        nearestDistanceSquared = distanceSquared;
                    }
                }

                return closestKnot;
            }

            onPressed: (me) => {
                if (root.readOnly) {
                    return;
                }

                root.forceActiveFocus();

                const closestKnot = mouseArea.closestKnotIndex(me.x, me.y);

                if (closestKnot === -1) {
                    root.selectedKnotIndex = root.curve.addPoint(Qt.point(me.x / (mouseArea.width - 1),
                                                                          1 - me.y / (mouseArea.height - 1)));
                } else {
                    root.selectedKnotIndex = closestKnot;
                }

                mouseArea.grabbedKnotOriginalPosition = root.curve.points[root.selectedKnotIndex];
                mouseArea.grabbedKnotOffset = Qt.point(me.x - mouseArea.grabbedKnotOriginalPosition.x
                                                            * (mouseArea.width - 1),
                                                       me.y - ((mouseArea.height - 1)
                                                            - mouseArea.grabbedKnotOriginalPosition.y
                                                            * (mouseArea.height - 1)));

                mouseArea.draggedAwayKnotIndex = -1;
                mouseArea.cursorShape = Qt.CrossCursor;
                mouseArea.isDragging = true;
            }

            onReleased: mouseArea.isDragging = false

            onPositionChanged: (me) => {
                if (root.readOnly) {
                    return;
                }

                if (!mouseArea.isDragging) {
                    const closestKnot = mouseArea.closestKnotIndex(me.x, me.y);
                    if (closestKnot < 0) {
                        mouseArea.cursorShape = Qt.ArrowCursor;
                    } else {
                        mouseArea.cursorShape = Qt.CrossCursor;
                    }
                } else {
                    const crossedHoriz = me.x - mouseArea.width > 15 || me.x < -15;
                    const crossedVert =  me.y - mouseArea.height > 15 || me.y < -15;

                    const removePoint = (crossedHoriz || crossedVert);

                    if (removePoint) {
                        if (mouseArea.draggedAwayKnotIndex === -1 && root.curve.points.length > 2) {
                            mouseArea.draggedAwayKnot = root.curve.points[root.selectedKnotIndex];
                            mouseArea.draggedAwayKnotIndex = root.selectedKnotIndex;
                            root.curve.removePoint(root.selectedKnotIndex);
                            if (root.selectedKnotIndex === root.curve.points.length) {
                                root.selectedKnotIndex = root.curve.points.length - 1;
                            }
                        }
                        return;
                    } else {
                        if (mouseArea.draggedAwayKnotIndex !== -1) {
                            root.curve.addPoint(mouseArea.draggedAwayKnot);
                            root.selectedKnotIndex = mouseArea.draggedAwayKnotIndex;
                            mouseArea.draggedAwayKnotIndex = -1;
                        }
                    }

                    let x = (me.x - mouseArea.grabbedKnotOffset.x) / (mouseArea.width - 1);
                    let y = 1.0 - (me.y - mouseArea.grabbedKnotOffset.y) / (mouseArea.height - 1);

                    let leftX;
                    let rightX;
                    if (root.selectedKnotIndex === 0) {
                        leftX = 0.0;
                        if (root.curve.points.length > 1) {
                            rightX = root.curve.points[root.selectedKnotIndex + 1].x - 0.0001;
                        } else {
                            rightX = 1.0;
                        }
                    } else if (root.selectedKnotIndex === root.curve.points.length - 1) {
                        leftX = root.curve.points[root.selectedKnotIndex - 1].x + 0.0001;
                        rightX = 1.0;
                    } else {
                        leftX = root.curve.points[root.selectedKnotIndex - 1].x + 0.0001;
                        rightX = root.curve.points[root.selectedKnotIndex + 1].x - 0.0001;
                    }

                    x = Math.max(leftX, Math.min(x, rightX));
                    y = Math.max(0, Math.min(y, 1));

                    root.curve.setPoint(root.selectedKnotIndex, Qt.point(x, y));
                }
            }
        }
    }

    Keys.onPressed: (ke) => {
        if (ke.key == Qt.Key_Right) {
            if (ke.modifiers & Qt.ControlModifier) {
                if (root.selectedKnotIndex < root.curve.points.length - 1) {
                    root.selectedKnotIndex++;
                } 
            } else {
                let newPoint = root.curve.points[root.selectedKnotIndex];
                const maxX = root.selectedKnotIndex < root.curve.points.length - 1
                                ? root.curve.points[root.selectedKnotIndex + 1].x - 0.0001
                                : 1.0;
                if (maxX > newPoint.x) {
                    const inc = ke.modifiers & Qt.ShiftModifier ? 0.001 : 0.01;
                    newPoint.x = Math.min(newPoint.x + inc, maxX);
                    root.curve.setPoint(root.selectedKnotIndex, newPoint);
                }
            }
            ke.accepted = true;

        } else if (ke.key == Qt.Key_Left) {
            if (ke.modifiers & Qt.ControlModifier) {
                if (root.selectedKnotIndex > 0) {
                    root.selectedKnotIndex--;
                } 
            } else {
                let newPoint = root.curve.points[root.selectedKnotIndex];
                const minX = root.selectedKnotIndex > 0
                                ? root.curve.points[root.selectedKnotIndex - 1].x + 0.0001
                                : 0.0;
                if (minX < newPoint.x) {
                    const inc = ke.modifiers & Qt.ShiftModifier ? 0.001 : 0.01;
                    newPoint.x = Math.max(newPoint.x - inc, minX);
                    root.curve.setPoint(root.selectedKnotIndex, newPoint);
                }
            }
            ke.accepted = true;

        } else if (ke.key == Qt.Key_Up) {
            let newPoint = root.curve.points[root.selectedKnotIndex];
            if (newPoint.y < 1.0) {
                const inc = ke.modifiers & Qt.ShiftModifier ? 0.001 : 0.01;
                newPoint.y = Math.min(newPoint.y + inc, 1.0);
                root.curve.setPoint(root.selectedKnotIndex, newPoint);
            }
            ke.accepted = true;

        } else if (ke.key == Qt.Key_Down) {
            let newPoint = root.curve.points[root.selectedKnotIndex];
            if (newPoint.y > 0.0) {
                const inc = ke.modifiers & Qt.ShiftModifier ? 0.001 : 0.01;
                newPoint.y = Math.max(newPoint.y - inc, 0.0);
                root.curve.setPoint(root.selectedKnotIndex, newPoint);
            }
            ke.accepted = true;

        } else if (ke.key == Qt.Key_Delete || ke.key == Qt.Key_Backspace) {
            if (root.selectedKnotIndex > 0 && root.selectedKnotIndex < root.curve.points.length - 1) {
                const previouslySelectedKnotX = root.curve.points[root.selectedKnotIndex].x;

                root.curve.removePoint(root.selectedKnotIndex);
                if (Math.abs(root.curve.points[root.selectedKnotIndex].x - previouslySelectedKnotX) >
                    Math.abs(root.curve.points[root.selectedKnotIndex - 1].x - previouslySelectedKnotX)) {
                    root.selectedKnotIndex--;
                }
                mouseArea.cursorShape = Qt.ArrowCursor;
                mouseArea.isDragging = false;
            }
            ke.accepted = true;
            
        } else if (ke.key == Qt.Key_Escape && mouseArea.isDragging) {
            if (mouseArea.draggedAwayKnotIndex !== -1) {
                root.curve.addPoint(mouseArea.draggedAwayKnot);
                root.selectedKnotIndex = mouseArea.draggedAwayKnotIndex;
                mouseArea.draggedAwayKnotIndex = -1;
            }
            root.curve.setPoint(root.selectedKnotIndex, mouseArea.grabbedKnotOriginalPosition);
            mouseArea.cursorShape = Qt.ArrowCursor;
            mouseArea.isDragging = false;
            ke.accepted = true;

        } else if ((ke.key == Qt.Key_A || ke.key == Qt.Key_Insert) && !mouseArea.isDragging) {
            const selectedKnotX = root.curve.points[root.selectedKnotIndex].x;
            const neighborKnotX = root.selectedKnotIndex === root.curve.points.length - 1
                                    ? root.curve.points[root.selectedKnotIndex - 1].x
                                    : root.curve.points[root.selectedKnotIndex + 1].x;
            if (Math.abs(selectedKnotX - neighborKnotX) <= 0.0002) {
                return;
            }
            const centerPointX = (selectedKnotX + neighborKnotX) / 2;
            const centerPointY = root.curve.value(centerPointX);
            root.selectedKnotIndex = root.curve.addPoint(Qt.point(centerPointX, centerPointY));
            ke.accepted = true;
        }
    }

    Connections {
        target: root.curve
        function onPointsChanged(pts) {
            contentItem.updatePoints();
        }
    }
}
