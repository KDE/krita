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
                i18nc("@label:inlistbox", "None"),
                i18nc("@label:inlistbox", "Capitalize"),
                i18nc("@label:inlistbox", "UpperCase"),
                i18nc("@label:inlistbox", "Lowercase")
            ]
            Layout.fillWidth: true
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
