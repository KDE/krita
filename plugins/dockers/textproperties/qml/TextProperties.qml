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

    TabBar {
        id: tabs
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: parent.top;
        palette: paletteControl.palette;
        TabButton {
            text: i18nc("@title:tab", "Character")
        }
        TabButton {
            text: i18nc("@title:tab", "Paragraph")
        }
        TabButton {
            text: i18nc("@title:tab", "Preset")
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

        ResourceView {
            id: presetView;
            resourceType: "css_styles";
            Layout.fillHeight: true;
            Layout.fillWidth: true;
            palette: paletteControl.palette;

            onCurrentResourceChanged: {
                mainWindow.applyPreset(currentResource);
            }

            resourceDelegate: ItemDelegate {
                required property var model;
                width: ListView.view.width;
                text: model.name;

                onClicked: presetView.currentIndex = model.index;
            }
        }
    }
}
