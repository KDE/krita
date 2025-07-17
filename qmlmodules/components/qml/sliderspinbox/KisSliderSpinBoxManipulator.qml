/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15

FocusScope {
    id: root

    property real min: 0.0
    property real max: 0.0
    property real value: 0.0
    property real stepSize: 1.0
    property bool blockUpdateSignalOnDrag: false
    property real exponentRatio: 1.0
    property real fastSliderStep: 5.0
    readonly property alias dragging : mouseArea.isDragging
    property int focusPolicy: Qt.WheelFocus

    signal editingStarted()
    signal editingStartedWithValue(v: real)
    signal increase(wheel: bool)
    signal decrease(wheel: bool)
    
    MouseArea {
        id: mouseArea

        property bool isDragging: false
        property bool useRelativeDragging: false
        property real relativeDraggingOffset: 0.0
        property point lastMousePressPosition: Qt.point(0.0, 0.0)
        property real cumulatedTrackpadLength: 0.0

        function pointForValue() : real
        {
            const rangeSize = root.max - root.min;
            if (rangeSize <= 0.0) {
                return 0.0;
            }
            const localPosition = root.value - root.min;
            const normalizedValue = Math.pow(localPosition / rangeSize, 1.0 / root.exponentRatio);
            return Math.max(0.0, Math.min(width, Math.round(normalizedValue * width)));
        }

        function valueForPoint(x: real, y: real, modifiers: int) : real
        {
            const center = Qt.point(
                mouseArea.lastMousePressPosition.x + (mouseArea.useRelativeDragging ? mouseArea.relativeDraggingOffset : 0),
                mouseArea.height / 2.0
            );
            const rangeSize = root.max - root.min;
            const distanceY =
                Math.max(
                    0.0,
                    Math.abs(y - center.y) - center.y - 32.0
                );
            const scale =
                modifiers & Qt.ShiftModifier
                ? (mouseArea.width + 2.0 * distanceY * 10.0) / mouseArea.width + 4.0
                : (mouseArea.width + 2.0 * distanceY * 2.0) / mouseArea.width;
            const scaledRectLeft = (0.0 - center.x) * scale + center.x;
            const scaledRectRight = (mouseArea.width - center.x) * scale + center.x;
            const scaledRectWidth = scaledRectRight - scaledRectLeft;
            const posX = x - scaledRectLeft;
            const normalizedPosX = Math.max(0.0, Math.min(1.0, posX / scaledRectWidth));
            const normalizedValue = Math.pow(normalizedPosX, root.exponentRatio);
            let v = normalizedValue * rangeSize + root.min;
            if (modifiers & Qt.ControlModifier) {
                v = Math.round(v / root.fastSliderStep) * root.fastSliderStep;
            }
            return v;
        }

        anchors.fill: parent
        cursorShape: Qt.SplitHCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        focus: true
        activeFocusOnTab: true
        preventStealing: true

        onPressAndHold: {
            if (isDragging) {
                return;
            }
            root.editingStarted()
        }

        onPressed: (me) => {
            if (root.focusPolicy & Qt.ClickFocus) {
                mouseArea.forceActiveFocus(Qt.MouseFocusReason);
            }

            if (me.button !== Qt.LeftButton) {
                return;
            }

            mouseArea.lastMousePressPosition = Qt.point(me.x, me.y);
            mouseArea.relativeDraggingOffset = mouseArea.pointForValue() - me.x;
            mouseArea.useRelativeDragging = (me.modifiers & Qt.ShiftModifier);
        }

        onReleased: (me) => {
            if (me.button === Qt.RightButton) {
                root.editingStarted();

            } else if (me.button === Qt.LeftButton) {
                if (root.blockUpdateSignalOnDrag) {
                    let x = mouseArea.useRelativeDragging ? me.x + mouseArea.relativeDraggingOffset : me.x;
                    value = mouseArea.valueForPoint(x, me.y, me.modifiers);
                } else {
                    if (!mouseArea.isDragging) {
                        root.value = mouseArea.valueForPoint(me.x, me.y, me.modifiers);
                    }
                }
                mouseArea.isDragging = false;
            }
        }

        onPositionChanged: (me) => {
            if (!(me.buttons & Qt.LeftButton)) {
                return;
            }
            if (!mouseArea.isDragging) {
                const dx = me.x - mouseArea.lastMousePressPosition.x;
                const dy = me.y - mouseArea.lastMousePressPosition.y;
                if (dx * dx + dy * dy <= 2 * 2) {
                    return;
                } else {
                    mouseArea.isDragging = true;
                }
            }
            const x = mouseArea.useRelativeDragging ? me.x + mouseArea.relativeDraggingOffset : me.x;
            root.value = mouseArea.valueForPoint(x, me.y, me.modifiers);
        }

        onWheel: (we) => {
            if (root.focusPolicy & Qt.WheelFocus) {
                mouseArea.forceActiveFocus(Qt.MouseFocusReason);
            }

            let inc = 0.0;
            if (we.pixelDelta && we.pixelDelta.y !== 0) {
                mouseArea.cumulatedTrackpadLength += we.pixelDelta.y;
                if (Math.abs(mouseArea.cumulatedTrackpadLength) > 30) {
                    inc = mouseArea.cumulatedTrackpadLength;
                    mouseArea.cumulatedTrackpadLength = 0.0;
                }
            } else {
                inc = we.angleDelta.y;
            }

            inc *= we.inverted ? -1 : 1;
            if (inc > 0) {
                root.increase(true);
            } else if (inc < 0) {
                root.decrease(true);
            }
        }

        Keys.onPressed: (ke) => {
            if (ke.key === Qt.Key_Right) {
                root.increase(false);
            } else if (ke.key === Qt.Key_Left) {
                root.decrease(false);
            } else if (ke.key === Qt.Key_Return || ke.key === Qt.Key_Enter) {
                root.editingStarted();
            } else if (ke.key === Qt.Key_Up || ke.key === Qt.Key_Down || ke.key === Qt.Key_Escape) {
                return;
            } else if (ke.key >= Qt.Key_0 && ke.key <= Qt.Key_9) {
                root.editingStartedWithValue(ke.key - Qt.Key_0);
            }
        }
    }
}
