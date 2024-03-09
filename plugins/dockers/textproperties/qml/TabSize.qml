/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

TextPropertyBase {
    GridLayout {
        columns: 3
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Label {
            text: "Tab Size:"
            Layout.columnSpan: 2;
            Layout.fillWidth: true;
        }

        Item {
            width: 1;
            height: 1;
        }

        SpinBox {
            id: tabSizeSpn;
            Layout.fillWidth: true;
        }
        ComboBox {
            model: ["Spaces", "Pt", "Em", "Ex"]
        }

    }
}
