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

    SystemPalette {
        id: sysPalette;
        colorGroup: root.enabled?
                        root.activeFocus? SystemPalette.Active:
                                          SystemPalette.Inactive: SystemPalette.Disabled;
    }

    property int groupPosition: KisGroupButton.GroupPosition.NoGroup
    palette.button: checked? sysPalette.highlight: sysPalette.button;
    palette.buttonText: checked? sysPalette.highlightedText: sysPalette.buttonText

    text: "Button"
    clip: true
    implicitWidth: implicitContentWidth + (root.leftPadding ?? root.padding)
                                        + (root.rightPadding ?? root.padding)
    leftInset: (groupPosition === KisGroupButton.GroupPosition.GroupRight ||
                groupPosition === KisGroupButton.GroupPosition.GroupCenter) * -50
    rightInset: (groupPosition === KisGroupButton.GroupPosition.GroupLeft ||
                 groupPosition === KisGroupButton.GroupPosition.GroupCenter) * -50

    Rectangle {
        width: 1
        height: root.background.height
        anchors.right: parent.right
        color: root.palette.buttonText;
        opacity: 0.1;
        visible: root.groupPosition === KisGroupButton.GroupPosition.GroupLeft ||
                 root.groupPosition === KisGroupButton.GroupPosition.GroupCenter
    }
}
