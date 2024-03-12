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
            text: i18nc("@title:group", "Glyphs: East-Asian:")
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@label:listbox", "Variant:")
        }
        ComboBox {
            model: [
                i18nc("@label:inlistbox", "Normal"),
                i18nc("@label:inlistbox", "JIS78"),
                i18nc("@label:inlistbox", "JIS83"),
                i18nc("@label:inlistbox", "JIS90"),
                i18nc("@label:inlistbox", "JIS04"),
                i18nc("@label:inlistbox", "Simplified"),
                i18nc("@label:inlistbox", "Traditional")
            ]
            Layout.fillWidth: true;
            id: variantCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        Label {
            text: i18nc("@label:listbox", "Width:")
        }
        ComboBox {
            model: [
                i18nc("@label:inlistbox", "Normal"),
                i18nc("@label:inlistbox", "Full width"),
                i18nc("@label:inlistbox", "Proportional")
            ]
            Layout.fillWidth: true;
            id: widthCmb;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
            Layout.columnSpan: 2;
        }
        CheckBox {
            text: i18nc("@option:check", "Ruby")
            id: rubyCbx;
        }

    }
}
