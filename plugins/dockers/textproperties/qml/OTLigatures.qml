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
        columns: 2;
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
            text: i18nc("@title:group", "Glyphs: Ligatures:")
            Layout.fillWidth: true;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Common")
            id: commonLigaCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Discretionary")
            id: discretionaryLigaCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Historical")
            id: historicalLigaCbx;
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        CheckBox {
            text: i18nc("@option:check", "Contextual")
            id: contextualLigaCbx;
        }

    }
}
