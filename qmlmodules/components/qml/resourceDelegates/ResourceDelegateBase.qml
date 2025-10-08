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
    /*
        \qmlProperty model
        value automatically gained from the model and represents the model data.
     */
    required property var model;
    /*
        \qmlProperty resourceView
        the resource view this is set on.
     */
    property Kis.ResourceView resourceView;

    signal resourceLeftClicked();
    signal resourceDoubleClicked();

    highlighted: resourceView.highlightedIndex === model.index;

    /*
        \qmlProperty selected
        Whether the current item is selected in the model wrapper.
     */
    property bool selected: resourceView.modelWrapper.currentIndex === model.index;

    palette: resourceView.palette;
}
