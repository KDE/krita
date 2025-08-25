/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0 as Kis

ToolTip {
    id: root

    delay: Qt.styleHints.mousePressAndHoldInterval;
    timeout: Qt.styleHints.mousePressAndHoldInterval*10;

    property Item parentControl
    property bool changingParentControl: false

    onParentChanged: {
        if (changingParentControl) {
            return;
        }
        if (parentControl) {
            parentControl = undefined;
            root.visible = false;
        }
    }

    Kis.Theme {
        id: theme;
    }

    palette.toolTipBase: theme.tooltip.backgroundColor;
    palette.toolTipText: theme.tooltip.textColor;
    palette.link: theme.tooltip.linkTextColor;
    palette.linkVisited: theme.tooltip.linkVisitedColor;

    palette.light: theme.tooltip.lightShadeColor;
    palette.midlight: theme.tooltip.midLightShadeColor;
    palette.mid: theme.tooltip.midShadeColor;
    palette.dark: theme.tooltip.darkShadeColor;
    palette.shadow: theme.tooltip.shadowShadeColor;

    onParentControlChanged: {
        changingParentControl = true;
        if (root.parentControl) {
            root.parent = root.parentControl;
            root.parentControl.hoverEnabled = true;
            root.visible = Qt.binding(() => { return root.parentControl.hovered });
        } else {
            root.parent = undefined;
            root.visible = false;
        }
        changingParentControl = false;
    }
}
