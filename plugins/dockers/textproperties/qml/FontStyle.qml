/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12

CollapsibleGroupProperty {
    propertyName: "Font Style";

    property alias fontWeight: fontWeightSpn.value;
    property alias fontWidth: fontStretchSpn.value;
    property alias fontSlant: fontSlantCmb.currentIndex;
    property alias fontOptical: opticalSizeCbx.checked;

    titleItem: ComboBox {
        id: styleCmb;
        Layout.fillWidth: true;
    }

    contentItem: GridLayout {
        columns: 3
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: 5;

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Label {
            text: "Weight:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            horizontalAlignment: Text.AlignRight;
        }

        SpinBox {
            id: fontWeightSpn
            from: 0;
            to: 1000;
            Layout.fillWidth: true
        }


        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Label {
            text: "Width:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            horizontalAlignment: Text.AlignRight;
        }

        SpinBox {
            id: fontStretchSpn
            from: 0;
            to: 200;
            Layout.fillWidth: true
        }

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Label {
            text: "Slant:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            horizontalAlignment: Text.AlignRight;
        }

        ComboBox {
            id: fontSlantCmb
            model: ["Normal", "Italic", "Oblique"]
            Layout.fillWidth: true
        }

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }
        Item {
        width: 1;
        height: 1;}
        CheckBox {
            id: opticalSizeCbx
            text: "Optical Size"
            Layout.fillWidth: true
        }

    }
}
