/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
/*
    \qmltype TextInputWithPrefixAndSuffix
    This is a Text Input which holds a prefix and a suffix.

 */
TextInputWithSelectionRange {
    /*
        \qmlproperty prefix
        A string that will be prefixed to the current input.
    */
    property string prefix: ""
    /*
        \qmlproperty suffix
        A string that will be suffixed to the current input.
    */
    property string suffix: ""
    /*
        \qmlproperty contentsText
        The current input text.
    */
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
