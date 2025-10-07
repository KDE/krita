/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

Control {
    id: root;
    anchors.fill: parent;

    property TextPropertiesCanvasObserver canvasObserver: TextPropertiesCanvasObserver {
    };

    Connections {
        target: canvasObserver;
        function onTextPropertyConfigChanged() {updatePropertyVisibilityState()}
        function onTextPropertiesChanged() {setProperties()}

    }

    Kis.WindowFocusChecker {
        id: focusChecker;
        onInFocusChanged: canvasObserver.hasFocus = inFocus;
    }

    property TextPropertyConfigModel configModel : canvasObserver.textPropertyConfig;
    property double canvasDPI: canvasObserver.dpi;
    Kis.ThemedControl {
        id: pal;
    }
    palette: pal.palette;
    background: Rectangle {
        color: palette.window;
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

    onCanvasObserverChanged: updatePropertyVisibilityState();
    Component.onCompleted: {
        updatePropertyVisibilityState();
        setProperties();
    }

    TabBar {
        id: tabs
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: parent.top;
        Kis.TabButtonBase {
            text: i18nc("@title:tab", "Character")
        }
        Kis.TabButtonBase {
            text: i18nc("@title:tab", "Paragraph")
        }
        Kis.TabButtonBase {
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
            canvasDPI: root.canvasDPI;
            locales: canvasObserver.locales;
            propertiesModel: canvasObserver.textProperties;

            onCallPropertyVisibilityConfig: canvasObserver.callModalTextPropertyConfigDialog();
        }

        TextPropertyBaseList {
            id: paragraphPropertyList;
            propertyType: TextPropertyConfigModel.Paragraph;
            configModel: root.configModel;
            canvasDPI: root.canvasDPI;
            locales: canvasObserver.locales;
            propertiesModel: canvasObserver.textProperties;

            onCallPropertyVisibilityConfig: canvasObserver.callModalTextPropertyConfigDialog();
        }

        ColumnLayout {
            Layout.fillHeight: true;
            Layout.fillWidth: true;
            Item {
                id: topPadding;
                height: Math.floor(spacing/2);
                Layout.fillWidth: true;
            }

            Kis.ResourceView {
                id: presetView;
                resourceType: "css_styles";
                Layout.fillHeight: true;
                Layout.fillWidth: true;

                resourceDelegate: Kis.CssStylePresetDelegate {
                    resourceView: presetView;
                    onResourceDoubleClicked: {
                        canvasObserver.applyPreset(presetView.modelWrapper.currentResource);
                    }
                }
                additionalAddResourceRow: RowLayout {
                    height: implicitHeight;
                    width: parent.width;
                    Layout.alignment: Qt.AlignRight;

                    Item {
                        Layout.fillWidth: true;
                    }

                    ToolButton {
                        text:  i18nc("@label:button", "Add Style Preset");
                        display: AbstractButton.IconOnly;
                        icon.source: "qrc:///light_list-add.svg";
                        icon.color: palette.text;
                        icon.width: 16;
                        icon.height: 16;
                        onClicked: canvasObserver.createNewPresetFromSettings();
                        hoverEnabled: true;
                        ToolTip.text: text;
                        ToolTip.visible: hovered;
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
                    }
                    ToolButton {
                        text:  i18nc("@label:button", "Edit Style Preset");
                        display: AbstractButton.IconOnly;
                        icon.source: "qrc:///16_light_edit-rename.svg";
                        icon.color: palette.text;
                        icon.width: 16;
                        icon.height: 16;
                        onClicked: canvasObserver.editPreset(presetView.modelWrapper.currentResource);
                        hoverEnabled: true;
                        Kis.ThemedControl {
                            id: editPalette;
                        }
                        palette: editPalette.palette;

                        enabled: presetView.modelWrapper.currentIndex >= 0;
                        ToolTip.visible: hovered;
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
                        ToolTip.text: text;
                    }
                    ToolButton {
                        text:  i18nc("@label:button", "Clone Style Preset");
                        display: AbstractButton.IconOnly;
                        icon.source: "qrc:///light_edit-copy.svg";
                        icon.color: palette.text;
                        icon.width: 16;
                        icon.height: 16;
                        onClicked: canvasObserver.cloneAndEditPreset(presetView.modelWrapper.currentResource);
                        hoverEnabled: true;
                        Kis.ThemedControl {
                            id: cloneButtonPalette;
                        }
                        palette: cloneButtonPalette.palette;

                        enabled: presetView.modelWrapper.currentIndex >= 0;
                        ToolTip.visible: hovered;
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;
                        ToolTip.text: text;
                    }
                }
            }
        }
    }
}
