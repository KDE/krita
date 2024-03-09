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
            text: "Text Decoration:"
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: "Line:"
            Layout.columnSpan: 2;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: "Underline"
            id: underlineCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: "Overline"
            id: overlineCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: "Line-through"
            id: linethoughCbx;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        CheckBox {
            text: "Color"
            id: colorCbx;
        }
        Rectangle {
            // replace with color picker?
            Layout.fillWidth: true;
            Layout.fillHeight: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: "Style:"
        }

        ComboBox {
            model: ["Solid", "Dotted", "Dashed", "Double", "Wavy"]
            Layout.fillWidth: true;
        }
    }
}
