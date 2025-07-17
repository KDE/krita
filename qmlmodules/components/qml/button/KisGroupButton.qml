/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: root

    enum GroupPosition {
        NoGroup,
        GroupLeft,
        GroupRight,
        GroupCenter
    }

    property int groupPosition: KisGroupButton.GroupPosition.NoGroup
    property color normalButtonColor
    property color normalButtonTextColor

    function fixPalette()
    {
        if (root.checked) {
            root.palette.active.button = root.palette.active.highlight;
            root.palette.active.buttonText = root.palette.active.highlightedText;
        } else {
            root.palette.active.button = normalButtonColor;
            root.palette.active.buttonText = normalButtonTextColor;
        }
        root.palette.inactive.button = Qt.darker(root.palette.active.button, 1.1);
        root.palette.inactive.buttonText = Qt.darker(root.palette.active.buttonText, 1.1);
        root.palette.disabled.button = Qt.alpha(root.palette.active.button, 0.5);
        root.palette.disabled.buttonText = Qt.alpha(root.palette.active.buttonText, 0.5);
    }

    function updatePalette()
    {
        root.normalButtonColor = root.palette.active.button
        root.normalButtonTextColor = root.palette.active.buttonText
        fixPalette();
    }

    text: "Button"
    clip: true
    padding: 8
    implicitWidth: implicitContentWidth + (root.leftPadding ?? root.padding)
                                        + (root.rightPadding ?? root.padding)
    leftInset: (groupPosition === KisGroupButton.GroupPosition.GroupRight ||
                groupPosition === KisGroupButton.GroupPosition.GroupCenter) * -50
    rightInset: (groupPosition === KisGroupButton.GroupPosition.GroupLeft ||
                 groupPosition === KisGroupButton.GroupPosition.GroupCenter) * -50

    Component.onCompleted: {
        updatePalette();
        root.checkedChanged.connect(() => { root.fixPalette(); });
        KisUI.paletteChanged.connect(updatePalette);
    }

    Rectangle {
        width: 1
        height: root.background.height
        anchors.right: parent.right
        color: Qt.alpha(root.palette.buttonText, 0.1)
        visible: root.groupPosition === KisGroupButton.GroupPosition.GroupLeft ||
                 root.groupPosition === KisGroupButton.GroupPosition.GroupCenter
    }
}
