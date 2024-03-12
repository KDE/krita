/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

CollapsibleGroupProperty {
    propertyName: i18nc("@label", "Font Family:");

    property var fontFamilies: [];
    property var fontFamilyModel : [];
    onFontFamiliesChanged: {
        if (fontFamilies.length >0) {
            mainFamilyCmb.currentIndex = mainFamilyCmb.find(fontFamilies[0]);
        }
        familyListView.model = fontFamilies;
        height: childrenRect.height;
    }

    titleItem: ComboBox {
        id: mainFamilyCmb;
        model: fontFamilyModel;
        Layout.fillWidth: true;
        onActivated: {if (fontFamilies.length >0) {
                fontFamilies[0] = currentText;
            }
        }
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
            Layout.preferredHeight: ItemDelegate.implicitHeight*3;
            background: Rectangle {
                color: sysPalette.alternateBase;
                border.color: sysPalette.text;
                border.width: 1;
            }
            ListView {
                id: familyListView;
                anchors.fill: parent;
                model: []
                delegate: ItemDelegate {
                    text: modelData;
                    width: parent.width;
                }
            }
        }


    }
}

