/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0 as Kis

/*
    \qmltype GroupButton

    A thin tool button which can be visually grouped with other buttons.

    The group can thus look like one solid bar: ( button1 | button2 | button3 )

    \qml
    Row {
        ButtonGroup {
            id: alignGroup;
        }
        GroupButton {
            icon.source: "qrc:///16_light_format-justify-left.svg";
            icon.width: 16;
            icon.height: 16;
            text: i18nc("@action:button", "Align Left");

            groupPosition: GroupButton.GroupLeft;
            ButtonGroup.group: alignGroup;
        }
        GroupButton {
            icon.source: "qrc:///16_light_format-justify-center.svg";
            icon.width: 16;
            icon.height: 16;
            text: i18nc("@action:button", "Align Center");

            groupPosition: GroupButton.GroupCenter;
            ButtonGroup.group: alignGroup;
        }
        GroupButton {
            icon.source: "qrc:///16_light_format-justify-right.svg";
            icon.width: 16;
            icon.height: 16;
            text: i18nc("@action:button", "Align Right");

            groupPosition: GroupButton.GroupRight;
            ButtonGroup.group: alignGroup;
        }
    }
    \endqml
 */
Button {
    id: root

    enum GroupPosition {
        NoGroup,    //!< No particular position, gives the button unchanged appearance
        GroupLeft,  //!< The button is at the left of the group, so it would have rounded the left part
        GroupRight, //!< The button is at the right of the group, so it would have rounded the right part
        GroupCenter //!< The button is on the center of the group, so it would have separators on both sides
    }

    Kis.Theme {
        id: theme;
        button.state: root.enabled? root.activeFocus? SystemPalette.Active: SystemPalette.Inactive: SystemPalette.Disabled;
    }

    /*
        \qmlproperty groupPosition
        One of the GroupPosition enum.

        /sa GroupPosition
    */
    property int groupPosition: GroupButton.GroupPosition.NoGroup;
    palette.button: checked? theme.selection.backgroundColor: theme.button.backgroundColor;
    palette.buttonText: checked? theme.selection.textColor: theme.button.textColor;

    text: "Button"
    clip: true
    implicitWidth: implicitContentWidth + (root.leftPadding ?? root.padding)
                                        + (root.rightPadding ?? root.padding)
    leftInset: (groupPosition === GroupButton.GroupPosition.GroupRight ||
                groupPosition === GroupButton.GroupPosition.GroupCenter) * -50
    rightInset: (groupPosition === GroupButton.GroupPosition.GroupLeft ||
                 groupPosition === GroupButton.GroupPosition.GroupCenter) * -50

    Rectangle {
        width: 1
        height: root.background.height
        anchors.right: parent.right
        color: palette.buttonText;
        opacity: 0.1;
        visible: root.groupPosition === GroupButton.GroupPosition.GroupLeft ||
                 root.groupPosition === GroupButton.GroupPosition.GroupCenter
    }
}
