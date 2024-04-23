/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15

TextPropertyBase {
    id: root;
    property alias propertyName: propertyTitle.text;
    property alias contentItem: contentItemContainer.data;
    property alias titleItem: titleItemContainer.data;
    spacing: columnSpacing;

    SystemPalette {
        id: sysPalette;
    }

    GridLayout {
        id: grid;
        flow: GridLayout.LeftToRight
        columns: 2;
        columnSpacing: root.columnSpacing;

        Item {
            Layout.preferredHeight: firstColumnWidth;
            Layout.preferredWidth: firstColumnWidth;
            Layout.minimumHeight: firstColumnWidth;
            Layout.minimumWidth: firstColumnWidth;
            ToolButton {
                id: collapse;
                display: AbstractButton.IconOnly
                icon.source: contentItemContainer.visible? "qrc:///light_groupOpened.svg" : "qrc:///light_groupClosed.svg";
                icon.color: sysPalette.text;
                icon.width: 12;
                icon.height: 12;
                onClicked: contentItemContainer.visible = !contentItemContainer.visible;

                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: parent.verticalCenter;
            }
        }


        RowLayout {
            id: titleItemContainer;
            Layout.fillWidth: true;
            spacing: columnSpacing;
            Label {
                id: propertyTitle;
                text: "property name";
                height: parent.height;
                verticalAlignment: Text.AlignVCenter
                color: sysPalette.text;

            }
        }
    }

    Item {
        id: contentItemContainer;
        width: parent.width;
        height: visible? childrenRect.height: 0;
    }
}
