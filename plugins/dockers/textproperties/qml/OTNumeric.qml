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
        columns: 3;
        columnSpacing: columnSpacing;
        width: parent.width;

        Item {
            width: firstColumnWidth;
            height: firstColumnWidth;
            ToolButton {
                id: revert;
                icon.width: 22;
                icon.height: 22;
                display: AbstractButton.IconOnly
                icon.source: "qrc:///light_view-refresh.svg"
            }
        }

        Label {
            text: "Glyphs: Numeric:"
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: "Style:"
        }
        ComboBox {
            model: ["Normal", "Lining", "Oldstyle"]
            Layout.fillWidth: true;
            id: styleCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: "Spacing:"
        }
        ComboBox {
            model: ["Normal", "Proportional", "Tabular"]
            Layout.fillWidth: true;
            id: spacingCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: "Fractions:"
        }
        ComboBox {
            model: ["Normal", "Diagonal", "Stacked"]
            Layout.fillWidth: true;
            id: fractionCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {

            text: "Ordinals"
            id: ordinalsCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {

            text: "Slashed Zero"
            id: slashZeroCbx;
        }

    }
}
