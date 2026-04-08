/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.components 1.0 as Kis

/**
  \qmltype ResourceDelegateBase
  base for KoResource delegates.
  */
ItemDelegate {
    id: control;
    /*
        \qmlProperty model
        value automatically gained from the model and represents the model data.
     */
    required property var model;
    property int index: model.index;
    property double preferredHeight;
    property double minimumHeight;

    signal resourceLeftClicked();
    signal resourceDoubleClicked();

    /*
        \qmlProperty selected
        Whether the current item is selected in the model wrapper.
     */
    property bool selected: ListView.view? ListView.isCurrentItem: false;
    highlighted: ListView.view.highlightedIndex === index;

    palette: ListView.view.palette;

    onResourceLeftClicked: {
        ListView.view.resourceLeftClicked(control.index);
    }

    function resourceRightClicked(x, y, resourceName, resourceIndex) {
            ListView.view.openContextMenu(control, x, y, resourceName, resourceIndex);
    }

    function resourceHovered(hovered) {
        if (hovered && ListView.view) {
            ListView.view.highlightedIndex = index;
        }
    }

    contentItem: Label {
        palette: control.palette;
        text: control.model.name;
        elide: Text.ElideRight;
        anchors.fill: parent;
        color: control.highlighted? palette.highlightedText: palette.text;
    }
}
