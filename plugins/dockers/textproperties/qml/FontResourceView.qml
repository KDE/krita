/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0


ColumnLayout {
    id: root;
    clip:true;
    property var fontModel;
    property var tagModel;
    property alias currentIndex : view.currentIndex;
    property alias listWidth : view.width;

    RowLayout {
        id: tagAndConfig;

        ComboBox {
            id: tagFilter;
            model: tagModel;
            textRole: "display";
            Layout.fillWidth: true;
            onActivated: {
                mainWindow.slotFontTagActivated(currentIndex);
            }
        }

    }
    Frame {
        Layout.minimumHeight: font.pixelSize*3;
        Layout.preferredHeight: 300;
        Layout.maximumHeight: 500;
        Layout.fillWidth: true;
        clip: true;
        ListView {
            anchors.fill: parent;
            id: view;
            model: fontModel;
            ScrollBar.vertical: ScrollBar {
            }
        }
    }
    RowLayout {
        TextField {
            id: search;
            placeholderText: i18nc("@info:placeholder", "Search...");
            Layout.fillWidth: true;
            onTextEdited: mainWindow.slotFontSearchTextChanged(text);
        }
        CheckBox {
            id: opticalSizeCbx
            text: i18nc("@option:check", "Search in tag")
            onCheckedChanged: mainWindow.slotFontSearchInTag(checked);
        }
    }
}
