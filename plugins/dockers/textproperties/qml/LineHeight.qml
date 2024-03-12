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
    property alias isNormal: lineHeightNormalCbx.checked;
    property alias lineHeight: lineHeightSpn.value;
    property var lineHeightUnit;
    onLineHeightUnitChanged: lineHeightUnitCmb.currentIndex = lineHeightUnitCmb.indexOfValue(lineHeightUnit);

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
            text: i18nc("@label", "Line Height:")
            Layout.columnSpan: 2;
        }


        Item {
            width: firstColumnWidth;
            height: 1;
        }

        CheckBox {
            Layout.columnSpan: 2;
            text: i18nc("@option:check", "Normal")
            id: lineHeightNormalCbx;
            onCheckedChanged: {
                lineHeightSpn.enabled = !checked;
                lineHeightUnitCmb.enabled = !checked;
            }
        }
        Item {
            width: firstColumnWidth;
            height: 1;
        }
        SpinBox {
            id: lineHeightSpn
            Layout.fillWidth: true;
            enabled: !lineHeightNormalCbx.enabled;
        }

        ComboBox {
            id: lineHeightUnitCmb
            model: [
                { text: i18nc("@label:inlistbox", "Pt"), value: LineHeightModel.Pt},
                { text: i18nc("@label:inlistbox", "Em"), value: LineHeightModel.Em},
                { text: i18nc("@label:inlistbox", "Ex"), value: LineHeightModel.Ex},
                { text: i18nc("@label:inlistbox", "%"), value: LineHeightModel.Percentage},
                { text: i18nc("@label:inlistbox", "Lines"), value: LineHeightModel.Lines},
            ]
            textRole: "text";
            valueRole: "value";
            enabled: !lineHeightNormalCbx.enabled;
            onActivated: lineHeightUnit = lineHeightUnitCmb.currentValue;
        }
    }
}
