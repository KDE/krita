/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.components 1.0 as Kis

ItemDelegate {
    required property var model;
    property Kis.ResourceView resourceView;

    signal resourceLeftClicked();
    signal resourceDoubleClicked();

    highlighted: resourceView.highlightedIndex === model.index;
    property bool selected: resourceView.modelWrapper.currentIndex === model.index;

    palette: resourceView.palette;
}
