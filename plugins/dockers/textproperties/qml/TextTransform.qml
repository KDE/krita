/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

TextPropertyBase {
    property alias fullWidth: fullWidthChk.checked;
    property alias fullSizeKana: fullSizeKanaChk.checked;
    property int capitals;
    onCapitalsChanged: textTransformCaseCmb.currentIndex = textTransformCaseCmb.indexOfValue(capitals);
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
            text: i18nc("@title:group", "Text Transform:")
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        Label {
            text:  i18nc("@label:listbox", "Case:")
        }

        ComboBox {
            id: textTransformCaseCmb;
            model: [
                {text: i18nc("@label:inlistbox", "None"), value: 0},
                {text: i18nc("@label:inlistbox", "Capitalize"), value: 1},
                {text: i18nc("@label:inlistbox", "UpperCase"), value: 2},
                {text: i18nc("@label:inlistbox", "Lowercase"), value: 4}
            ]
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: capitals = currentValue;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {
            id: fullWidthChk;
            text: i18nc("@option:check", "Full-width")
            Layout.fillWidth: true;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {
            id: fullSizeKanaChk;
            text: i18nc("@option:check", "Full-size Kana")
            Layout.fillWidth: true;
        }
    }
}
