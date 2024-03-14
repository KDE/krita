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
    property int breakType;
    onBreakTypeChanged: lineBreakCmb.currentIndex = lineBreakCmb.indexOfValue(breakType);
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
            text: i18nc("@label:listbox", "Line Break:")
        }

        ComboBox {
            id: lineBreakCmb
            model: [
                { text: i18nc("@label:inlistbox", "Auto"), value: KoSvgText.LineBreakAuto},
                { text: i18nc("@label:inlistbox", "Loose"), value: KoSvgText.LineBreakLoose},
                { text: i18nc("@label:inlistbox", "Normal"), value: KoSvgText.LineBreakNormal},
                { text: i18nc("@label:inlistbox", "Strict"), value: KoSvgText.LineBreakStrict},
                { text: i18nc("@label:inlistbox", "Anywhere"), value: KoSvgText.LineBreakAnywhere}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: breakType = currentValue;
        }
    }
}
