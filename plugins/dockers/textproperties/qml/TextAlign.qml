/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

CollapsibleGroupProperty {
    propertyName: "Text Align";

    contentItem: GridLayout {
        columns: 3
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Label {
            text: i18nc("@label:listbox", "Text Align:")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        ComboBox {
            id: textAlignAllCmb;
            model: [
                {text: i18nc("@label:inlistbox", "Left")},
                {text: i18nc("@label:inlistbox", "Start")},
                {text: i18nc("@label:inlistbox", "Center")},
                {text: i18nc("@label:inlistbox", "End")},
                {text: i18nc("@label:inlistbox", "Right")},
                {text: i18nc("@label:inlistbox", "Justified")}
            ]
            Layout.fillWidth: true
        }


        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Label {
            text: i18nc("@label:listbox", "Text Anchor:")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        ComboBox {
            id: textAnchorCmb;
            model: [
                {text: i18nc("@label:inlistbox", "Start")},
                {text: i18nc("@label:inlistbox", "Middle")},
                {text: i18nc("@label:inlistbox", "End")}]
            Layout.fillWidth: true
        }

    }
}

