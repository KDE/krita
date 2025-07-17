/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15

TextInput {
    id: root

    property int selectionRangeStart: 0
    property int selectionRangeEnd: 0
    property int selectionRangeLength: selectionRangeEnd - selectionRangeStart
    property int displayTextWidth: textMetrics.boundingRect.width

    clip: displayTextWidth > width

    function clampSelection(selectionPosition: int)
    {
        return Math.min(Math.max(selectionPosition, selectionRangeStart), selectionRangeEnd);
    }

    onCursorPositionChanged: {
        if (cursorPosition != selectionStart && cursorPosition != selectionEnd) {
            cursorPosition = clampSelection(cursorPosition);
        } else if (cursorPosition != selectionEnd) {
            select(clampSelection(selectionEnd), clampSelection(selectionStart))
        } else {
            select(clampSelection(selectionStart), clampSelection(selectionEnd))
        }
    }
    
    Keys.onPressed: (event) => {
        if (event.key === Qt.Key_Backspace) {
            if (root.selectionRangeLength === 0 || root.cursorPosition === root.selectionRangeStart) {
                event.accepted = true
            }
        } else if (event.key === Qt.Key_Delete) {
            if (root.selectionRangeLength === 0 || root.cursorPosition === root.selectionRangeEnd) {
                event.accepted = true
            }
        }
    }

    Component.onCompleted: select(clampSelection(selectionStart), clampSelection(selectionEnd))

    TextMetrics {
        id: textMetrics
        font: root.font
        text: root.text
    }
}
