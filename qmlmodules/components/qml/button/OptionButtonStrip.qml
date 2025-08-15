/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

Row {
    id: root

    property alias exclusive: buttonGroup.exclusive
    readonly property Kis.GroupButton checkedButton: buttonGroup.checkedButton as Kis.GroupButton
    property int checkedButtonIndex: -1

    function updateCheckedStatusFromIndex()
    {
        if (!root.exclusive) {
            return;
        }

        if (root.checkedButtonIndex === -1) {
            for (let i = 0; i < root.children.length; ++i) {
                if (root.children[i] instanceof KisGroupButton) {
                    (root.children[i] as KisGroupButton).checked = false;
                }
            }
            return;
        }

        let buttonIndex = -1;
        for (let i = 0; i < root.children.length; ++i) {
            if (root.children[i] instanceof KisGroupButton) {
                ++buttonIndex;
                if (buttonIndex === root.checkedButtonIndex) {
                    (root.children[i] as KisGroupButton).checked = true;
                }
            }
        }
    }

    function buttonIndexFromCheckedButton() : int
    {
        if (!root.exclusive) {
            return -1;
        }
        let buttonIndex = -1;
        for (let i = 0; i < root.children.length; ++i) {
            if (root.children[i] instanceof KisGroupButton) {
                ++buttonIndex;
                if (root.children[i].checked) {
                    return buttonIndex;
                }
            }
        }
        return -1;
    }

    Component.onCompleted: {
        updateCheckedStatusFromIndex();
        root.exclusiveChanged.connect(() => { root.checkedButtonIndex = root.buttonIndexFromCheckedButton(); });
        root.checkedButtonChanged.connect(() => { root.checkedButtonIndex = buttonIndexFromCheckedButton(); });
        root.checkedButtonIndexChanged.connect(() => { root.updateCheckedStatusFromIndex(); });
    }

    ButtonGroup {
        id: buttonGroup
        buttons: root.children
        exclusive: true
    }
}
