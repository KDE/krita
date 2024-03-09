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
            text: "Glyphs: East-Asian:"
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: "Variant:"
        }
        ComboBox {
            model: ["Normal", "JIS78", "JIS83", "JIS90", "JIS04", "Simplified", "Traditional"]
            Layout.fillWidth: true;
            id: variantCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: "Width:"
        }
        ComboBox {
            model: ["Normal", "Full width", "Proportional"]
            Layout.fillWidth: true;
            id: widthCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {
            text: "Ruby"
            id: rubyCbx;
        }

    }
}
