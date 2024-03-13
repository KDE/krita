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

    property int textAlignAll;
    onTextAlignAllChanged: textAlignAllCmb.currentIndex = textAlignAllCmb.indexOfValue(textAlignAll);
    property int textAlignLast;
    onTextAlignLastChanged: textAlignLastCmb.currentIndex = textAlignLastCmb.indexOfValue(textAlignLast);
    property int textAnchor;
    onTextAnchorChanged: textAnchorCmb.currentIndex = textAnchorCmb.indexOfValue(textAnchor);

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
                {text: i18nc("@label:inlistbox", "Left"), value: 3},
                {text: i18nc("@label:inlistbox", "Start"), value: 1},
                {text: i18nc("@label:inlistbox", "Center"), value: 5},
                {text: i18nc("@label:inlistbox", "End"), value: 2},
                {text: i18nc("@label:inlistbox", "Right"), value: 4},
                {text: i18nc("@label:inlistbox", "Justified"), value: 6}
            ]
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: textAlignAll = currentValue;
        }

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Label {
            text: i18nc("@label:listbox", "Align Last:")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        ComboBox {
            id: textAlignLastCmb;
            model: [
                {text: i18nc("@label:inlistbox", "Auto"), value: 0},
                {text: i18nc("@label:inlistbox", "Left"), value: 3},
                {text: i18nc("@label:inlistbox", "Start"), value: 1},
                {text: i18nc("@label:inlistbox", "Center"), value: 5},
                {text: i18nc("@label:inlistbox", "End"), value: 2},
                {text: i18nc("@label:inlistbox", "Right"), value: 4}
            ]
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: textAlignLast = currentValue;
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
                {text: i18nc("@label:inlistbox", "Start"), value: 0},
                {text: i18nc("@label:inlistbox", "Middle"), value: 1},
                {text: i18nc("@label:inlistbox", "End"), value: 2}]
            Layout.fillWidth: true
            textRole: "text";
            valueRole: "value";
            onActivated: textAnchor = currentValue;
        }

    }
}

