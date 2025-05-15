/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0


Rectangle {
    id: root;
    color: sysPalette.window;
    anchors.fill: parent;

    property TextPropertyConfigModel configModel : textPropertyConfigModel;

    SystemPalette {
        id: sysPalette;
        colorGroup: SystemPalette.Active
    }

    PaletteControl {
        id: paletteControl;
        colorGroup: root.enabled? SystemPalette.Active: SystemPalette.Disabled;
    }

    function setProperties() {
        characterPropertyList.updateProperties()
        paragraphPropertyList.updateProperties()
    }

    function updatePropertyVisibilityState() {
        configModel.loadFromConfiguration();
        characterPropertyList.updatePropertyVisibilityState();
        paragraphPropertyList.updatePropertyVisibilityState();
    }

    RowLayout {
        id: configLayout;
        height: implicitHeight;
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: parent.top;
        Item {
            Layout.fillWidth: true;
        }
        ToolButton {
            id: configButton;
            icon.source: "qrc:///light_configure.svg"
            icon.color: palette.text;
            icon.width: 16;
            icon.height: 16;
            text: i18nc("@label:button", "Configure");

            palette: paletteControl.palette;

            onClicked: mainWindow.callModalTextPropertyConfigDialog();
        }
    }

    TabBar {
        id: tabs
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: configLayout.bottom;
        palette: paletteControl.palette;
        TabButton {
            text: i18nc("@title:tab", "Character")
        }
        TabButton {
            text: i18nc("@title:tab", "Paragraph")
        }
    }

    StackLayout {
        currentIndex: tabs.currentIndex;
        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: tabs.bottom;

        TextPropertyBaseList {
            id: characterPropertyList;
            propertyType: TextPropertyConfigModel.Character;
            configModel: root.configModel;
        }

        TextPropertyBaseList {
            id: paragraphPropertyList;
            propertyType: TextPropertyConfigModel.Paragraph;
            configModel: root.configModel;
        }
    }
}
