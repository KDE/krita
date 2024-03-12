/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

TextPropertyBase {
    RowLayout {
        spacing: columnSpacing;
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
            text:  i18nc("@label:listbox", "Word Break:")
        }


        ComboBox {
            id: wordBreakCmb
            model: [
                i18nc("@label:inlistbox", "Normal"),
                i18nc("@label:inlistbox", "Keep-all"),
                i18nc("@label:inlistbox", "Break-all")
            ]
            Layout.fillWidth: true;
        }
    }
}
