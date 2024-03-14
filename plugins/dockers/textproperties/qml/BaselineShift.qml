/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

TextPropertyBase {
    property alias baselineShiftValue: baselineShiftSpn.value;
    property int baselineShiftMode;
    onBaselineShiftModeChanged: {
        baselineShiftCmb.currentIndex = baselineShiftCmb.indexOfValue(baselineShiftMode);
        baselineShiftSpn.enabled = baselineShiftMode == 3;
    }

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
            text: i18nc("@label", "Baseline-Shift:")
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        ComboBox {
            Layout.columnSpan: 2;
            model: [
                { text: i18nc("@label:inlistbox", "None"), value: KoSvgText.ShiftNone},
                { text: i18nc("@label:inlistbox", "Length"), value: KoSvgText.ShiftLengthPercentage},
                { text: i18nc("@label:inlistbox", "Super"), value: KoSvgText.ShiftSuper},
                { text: i18nc("@label:inlistbox", "Sub"), value: KoSvgText.ShiftSub}
            ]
            id: baselineShiftCmb;
            textRole: "text";
            valueRole: "value";
            onActivated: baselineShiftMode = currentValue;
        }

        Item {
            width: firstColumnWidth;
            height: 1;
        }
        SpinBox {
            id: baselineShiftSpn
            Layout.fillWidth: true;
            enabled: baselineShiftMode == 3;
        }

        ComboBox {
            model: ["Pt", "Em", "Ex"]
        }
    }
}
