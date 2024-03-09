/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

CollapsibleGroupProperty {
    propertyName: "Font Family:";

    titleItem: ComboBox {
        id: mainFamilyCmb;
        Layout.fillWidth: true;
    }

    contentItem: GridLayout {
        columns: 2
        anchors.left: parent.left
        anchors.right: parent.right
        columnSpacing: columnSpacing;

        ToolButton {
            width: firstColumnWidth;
            height: firstColumnWidth;
            display: AbstractButton.IconOnly
            icon.source: "qrc:///light_view-refresh.svg"
        }

        ScrollView {
            id: fullFamilyList;
            Layout.fillWidth: true;
            background: Rectangle {
                color: sysPalette.alternateBase;
                border.color: sysPalette.text;
                border.width: 1;
            }

            Layout.preferredHeight: childrenRect.height;
            ListView {
                anchors.fill: parent;
                model: ListModel {
                    ListElement {
                        text: "Font A";
                    }
                    ListElement {
                        text: "Font B";
                    }
                    ListElement {
                        text: "Font C";
                    }
                }
                delegate: ItemDelegate {
                    text: model.text;
                    width: parent.width;
                }
            }
        }


    }
}

