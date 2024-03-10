/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

TextPropertyBase {
    property alias writingMode: writingModeCmb.currentIndex;

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
            text: "Writing Mode:"
        }


        ComboBox {
            id: writingModeCmb
            model: ["Horizontal", "Vertical, Right to Left", "Vertical, Left To Right"]
            Layout.fillWidth: true;
        }
    }
}
