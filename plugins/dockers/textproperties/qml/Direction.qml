/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

TextPropertyBase {
    property int direction;
    onDirectionChanged: directionCmb.currentIndex = directionCmb.indexOfValue(direction)

    RowLayout {
        id: row
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
            text: i18nc("@label:listbox", "Direction:");
        }

        ComboBox {
            id: directionCmb
            Layout.fillWidth: true;
            model: [
                {text: i18nc("@label:inlistbox", "Left to Right"), value: 0},
                {text: i18nc("@label:inlistbox", "Right to Left"), value: 1}
            ]
            textRole: "text";
            valueRole: "value";
            onActivated: direction = currentValue;
        }
    }
}
