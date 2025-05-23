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

            signal applyPreset;
            onApplyPreset: mainWindow.applyPreset(currentResource);

            resourceDelegate: ItemDelegate {
                id: presetDelegate;
                required property var model;
                property var meta: presetView.modelWrapper.metadataForIndex(model.index);
                width: ListView.view.width;
                highlighted: delegateMouseArea.containsMouse;
                property bool selected: presetView.currentIndex === model.index;
                contentItem: KoShapeQtQuickLabel {
                    width: parent.width;
                    height: nameLabel.height * 5;
                    imageScale: 3;
                    imagePadding: nameLabel.height;

                    svgData: presetDelegate.meta.sample_svg;
                    foregroundColor: presetDelegate.highlighted? palette.highlightedText: palette.text;

                    Label {
                        id: nameLabel;
                        palette: presetDelegate.palette;
                        text: presetDelegate.model.name;
                        anchors.top: parent.top;
                        anchors.left: parent.left;
                        elide: Text.ElideRight;
                        width: parent.width;
                        color: presetDelegate.highlighted? palette.highlightedText: palette.text;
                    }
                }
                background: Rectangle {
                    color: presetDelegate.highlighted? presetDelegate.palette.highlight: "transparent";
                    border.color: presetDelegate.selected? presetDelegate.palette.highlight: presetDelegate.palette.base;
                    border.width: presetDelegate.selected? 2: 1;
                }

                MouseArea {
                    id: delegateMouseArea;
                    acceptedButtons: Qt.RightButton | Qt.LeftButton;
                    anchors.fill: parent;
                    hoverEnabled: true;
                    onClicked: {
                        if (mouse.button === Qt.RightButton) {
                            presetView.openContextMenu(mouse.x, mouse.y, parent.model.name, parent.model.index);
                        } else {
                            presetView.modelWrapper.currentIndex = parent.model.index;
                        }
                    }
                    onDoubleClicked: {
                        presetView.modelWrapper.currentIndex = parent.model.index;
                        presetView.applyPreset;
                    }

                    ToolTip.text: presetDelegate.model.name + "\n" + presetDelegate.meta.description;
                    ToolTip.visible: containsMouse;
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;

                }
            }
        }
    }
}
