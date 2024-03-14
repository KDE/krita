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
    property int writingMode;
    onWritingModeChanged: writingModeCmb.currentIndex = writingModeCmb.indexOfValue(writingMode);

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
            text: i18nc("@label:listbox", "Writing Mode:")
        }


        ComboBox {
            id: writingModeCmb
            model: [
                {text: i18nc("@label:inlistbox", "Horizontal"), value: KoSvgText.HorizontalTB},
                {text: i18nc("@label:inlistbox", "Vertical, Right to Left"), value: KoSvgText.VerticalRL},
                {text: i18nc("@label:inlistbox", "Vertical, Left To Right"), value: KoSvgText.VerticalLR}
            ]
            Layout.fillWidth: true;
            textRole: "text";
            valueRole: "value";
            onActivated: writingMode = currentValue;
        }
    }
}
