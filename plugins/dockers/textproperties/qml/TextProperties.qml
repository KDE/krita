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

        ResourceView {
            id: presetView;
            resourceType: "css_styles";
            Layout.fillHeight: true;
            Layout.fillWidth: true;

            signal applyPreset;
            onApplyPreset: {
                canvasObserver.applyPreset(modelWrapper.currentResource);
            }

            resourceDelegate: ItemDelegate {
                id: presetDelegate;
                required property var model;
                property var meta: model.metadata;
                width: ListView.view.width;
                highlighted: presetView.highlightedIndex === model.index;
                property bool selected: presetView.modelWrapper.currentIndex === model.index;
                palette: presetView.palette;
                contentItem: KoShapeQtQuickLabel {
                    implicitHeight: nameLabel.height * 4;
                    padding: nameLabel.height;
                    alignment: presetDelegate.meta.sample_align;
                    scalingType: presetDelegate.meta.style_type === "paragraph"?
                                     KoShapeQtQuickLabel.FitWidth: KoShapeQtQuickLabel.Fit;
                    svgData: presetDelegate.meta.sample_svg;
                    foregroundColor: presetDelegate.highlighted? palette.highlightedText: palette.text;

                    Label {
                        id: nameLabel;
                        palette: presetDelegate.palette;
                        text: presetDelegate.model.name;
                        anchors.top: parent.top;
                        anchors.left: parent.left;
                        anchors.right: parent.right;
                        elide: Text.ElideRight;
                        color: presetDelegate.highlighted? palette.highlightedText: palette.text;
                    }

                    Row {
                        anchors.bottom: parent.bottom;
                        anchors.left: parent.left;
                        anchors.right: parent.right;
                        ToolButton {
                            id: missingFamily;
                            visible: typeof presetDelegate.meta.primary_font_family !== 'undefined'?
                                     presetDelegate.meta.primary_font_family !== ""
                                     && mainWindow.wwsFontFamilyName(presetDelegate.meta.primary_font_family, true) === "": false;
                            icon.source: palette.window.hslLightness < 0.5? "qrc:///16_light_warning.svg": "qrc:///16_dark_warning.svg";
                            icon.height: 8;
                            icon.width: 8;
                        }
                    }
                }
                background: Rectangle {
                    color: presetDelegate.highlighted? presetDelegate.palette.highlight: "transparent";
                    border.color: presetDelegate.selected? presetDelegate.palette.highlight: presetDelegate.palette.mid;
                    border.width: presetDelegate.selected? 2: 1;
                }

                MouseArea {
                    id: delegateMouseArea;
                    acceptedButtons: Qt.RightButton | Qt.LeftButton;
                    anchors.fill: parent;
                    hoverEnabled: true;
                    preventStealing: true;
                    onClicked: {
                        if (mouse.button === Qt.RightButton) {
                            presetView.openContextMenu(mouse.x, mouse.y, parent.model.name, parent.model.index);
                        } else {
                            presetView.applyHighlightedIndex();
                        }
                    }
                    onDoubleClicked: {
                        presetView.applyHighlightedIndex();
                        presetView.applyPreset();
                    }
                    onContainsMouseChanged: {
                        if (containsMouse) {
                            presetView.highlightedIndex = parent.model.index;
                        }
                    }

                    readonly property string missingFontName : i18nc("%1 is name of a font family", "Font Family \"%1\" is missing on this machine. This style preset may not work correctly.", presetDelegate.meta.primary_font_family);
                    Kis.ToolTipBase {
                        text: (typeof presetDelegate.meta.description !== 'undefined'? presetDelegate.model.name + "\n" + presetDelegate.meta.description: presetDelegate.model.name) + (missingFamily.visible?  "\n"+ delegateMouseArea.missingFontName: "");
                        visible: delegateMouseArea.containsMouse;
                    }

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
