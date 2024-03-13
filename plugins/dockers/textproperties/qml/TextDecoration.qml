/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

TextPropertyBase {

    property alias underline: underlineCbx.checked;
    property alias overline: overlineCbx.checked;
    property alias linethrough: linethroughCbx.checked;
    property int lineStyle;
    property color lineColor;
    onLineStyleChanged: lineStyleCmb.currentIndex = lineStyleCmb.indexOfValue(lineStyle)
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
            text: i18nc("@title:group", "Text Decoration:")
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@title:group", "Line:")
            Layout.columnSpan: 2;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: i18nc("@option:check", "Underline")
            id: underlineCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: i18nc("@option:check", "Overline")
            id: overlineCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            Layout.columnSpan: 2;
            text: i18nc("@option:check", "Line-through")
            id: linethroughCbx;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        CheckBox {
            text: i18nc("@label:chooser", "Color")
            id: colorCbx;
            enabled: false;
        }
        Rectangle {
            // replace with color picker?
            color: lineColor;
            Layout.fillWidth: true;
            Layout.fillHeight: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@label:listbox", "Style:")
        }

        ComboBox {
            id: lineStyleCmb;
            model: [
                { text: i18nc("@label:inlistbox", "Solid"), value: 0},
                { text: i18nc("@label:inlistbox", "Dotted"), value: 2},
                { text: i18nc("@label:inlistbox", "Dashed"), value: 3},
                { text: i18nc("@label:inlistbox", "Double"), value: 1},
                { text: i18nc("@label:inlistbox", "Wavy"), value: 4},
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: lineStyle = currentValue;
        }
    }
}
