/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
KisTextInputWithSelectionRange {
    property string prefix: ""
    property string suffix: ""
    property string contentsText: ""

    selectionRangeStart: prefix.length
    selectionRangeEnd: text.length - suffix.length

    text: prefix + contentsText + suffix

    onTextChanged: {
        const startPos = text.startsWith(prefix) ? prefix.length : 0;
        const endPos = text.endsWith(suffix) ? text.length - suffix.length : text.length;
        contentsText = text.slice(startPos, endPos);
    }
}
