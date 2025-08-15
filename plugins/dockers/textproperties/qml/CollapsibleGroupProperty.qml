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
    property alias contentItem: contentItemContainer.data;
    property alias titleItem: titleItemContainer.data;
    spacing: columnSpacing;


    GridLayout {
        id: grid;
        flow: GridLayout.LeftToRight
        columns: 2;
        columnSpacing: root.columnSpacing;
        width: parent.width;

        Item {
            Layout.preferredHeight: firstColumnWidth;
            Layout.preferredWidth: firstColumnWidth;
            Layout.minimumHeight: firstColumnWidth;
            Layout.minimumWidth: firstColumnWidth;
            ToolButton {
                id: collapse;
                display: AbstractButton.IconOnly
                icon.source: contentItemContainer.visible? "qrc:///light_groupOpened.svg" : "qrc:///light_groupClosed.svg";
                icon.color: palette.text;
                icon.width: 12;
                icon.height: 12;
                onClicked: contentItemContainer.visible = !contentItemContainer.visible;

                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: parent.verticalCenter;
            }
        }
        Item {
            id: titleItemContainer;
            Layout.fillWidth: true;
            implicitHeight: childrenRect.height;
            height: implicitHeight;
        }
    }

    Item {
        id: contentItemContainer;
        width: parent.width;
        implicitHeight: childrenRect.height;
        height: visible? implicitHeight: 0;
        visible: false;
    }
}
