/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

TextPropertyBase {
    property int baselineSelection
    onBaselineSelectionChanged: baselineCmb.currentIndex = baselineCmb.indexOfValue(baselineSelection);
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
            text: i18nc("@label:listbox", "Dominant Baseline:")
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }
        ComboBox {
            model: [
                {text: i18nc("@label:inlistbox", "Baseline"), value: 0},
                {text: i18nc("@label:inlistbox", "Alphabetic"), value: 6},
                {text: i18nc("@label:inlistbox", "Ideographic"), value: 5},
                {text: i18nc("@label:inlistbox", "Central"), value: 9},
                {text: i18nc("@label:inlistbox", "Hanging"), value: 7},
                {text: i18nc("@label:inlistbox", "Middle"), value: 10},
                {text: i18nc("@label:inlistbox", "Mathematical"), value: 8},
                {text: i18nc("@label:inlistbox", "Text-Top"), value: 11},
                {text: i18nc("@label:inlistbox", "Text-Bottom"), value: 12}
            ]
            Layout.fillWidth: true;
            id: baselineCmb;
            textRole: "text";
            valueRole: "value";
            onActivated: baselineSelection = currentValue;
        }
    }
}
