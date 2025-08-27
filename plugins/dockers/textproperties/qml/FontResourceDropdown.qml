/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

Button {
    id: familyCmb;
    wheelEnabled: true;

    //--- Model setup. ---//

    property alias modelWrapper : resourceView.modelWrapper;

    Connections {
        target: modelWrapper;
        function onResourceFilenameChanged() {
            familyCmb.fontFileName = modelWrapper.resourceFilename;
        }
    }

    property var locales: [];
    property alias fontFileName: textInput.text;
    topPadding: 0;
    bottomPadding: 0;
    spacing: padding;
    leftPadding:  (familyCmb.mirrored ? padding + indicator.width + spacing : 0);
    rightPadding: (!familyCmb.mirrored ? padding + indicator.width + spacing : 0);

    Kis.ThemedControl {
        id: palControl;
    }
    palette: palControl.palette;

    /// Ideally we'd have the max popup height be Window-height - y-pos-of-item-to-window.
    /// But that's pretty hard to calculate (map to global is screen relative, but there's
    /// no way to get window.y relative to screen), so we'll just use 300 as the maximum.
    property int maxPopupHeight: Math.min(Window.height, 500) - height*3;

    contentItem: Kis.InformingTextInput {
        id: textInput;

        warnColor: palControl.theme.window.neutralBackgroundColor;
        warnColorBorder: palControl.theme.window.neutralTextColor;
        readOnly: !familyCmb.enabled;

        onEditingFinished: {
            if (familyCmbPopup.visible) {
                resourceView.applyHighlightedIndex();
            }

            testInput();
            familyCmb.activated();
        }
        onTextChanged: {
            testInput();
        }

        onTextEdited: {
            stopWarning();
            modelWrapper.searchText = text;
            resourceView.showTagging = false;
            resourceView.showSearch = false;
            familyCmbPopup.open();
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                selectAll();
            }
        }

        warningTimeOut: activeFocus? 1000: 0;

        function testInput() {
            let test = mainWindow.wwsFontFamilyName(text, true);

            if (test === "") {
                if (!activeFocus) {
                    startWarning();
                }
            } else {
                stopWarning();
                text = test;
                modelWrapper.resourceFilename = text;
            }
        }

        Keys.onDownPressed: {
            resourceView.downPress();
        }
        Keys.onUpPressed: {
            resourceView.upPress();
        }
    }

    indicator: Image {
        id: imgIndicator
        x: Math.round(familyCmb.mirrored ? familyCmb.padding : familyCmb.width - width - familyCmb.padding);
        y: Math.round(familyCmb.topPadding + (familyCmb.availableHeight - height) / 2);
        source: familyCmb.palette.button.hslLightness < 0.5? "qrc:///light_groupOpened.svg": "qrc:///dark_groupOpened.svg";
        width: 12;
        height: 12;
        clip: true;
    }

    onClicked: {
        if (familyCmbPopup.visible) {
            familyCmbPopup.close();
        } else {
            resourceView.showTagging = true;
            resourceView.showSearch = true;
            familyCmbPopup.open();
        }
    }
    signal activated();
    onActivated: {
        familyCmbPopup.close();
        modelWrapper.searchText = "";
    }

    //--- Delegate setup. ---//
    Component {
        id: fontDelegate
        ItemDelegate {
            /**
              Ideally we'd just use required properties for the metadata etc.
              However, when changing the modelwrapper in the font list,
              there's a short moment where the model data is unavailable, so
              instead we ensure we're not accessing undefined objects.

              Not great, would like a better solution...
              */
            id: fontDelegateItem;
            required property var model;
            property string name: typeof model.name !== "undefined"? model.name: "";
            property int index: model.index;
            property string fontName: name;
            property var metadata: model.metadata;
            // TODO: change this to use the text locale, if possible.
            property string sample: "";

            onMetadataChanged: {
                updateNameAndSample();
            }

            Component.onCompleted: {
                updateNameAndSample();
            }

            function updateNameAndSample() {
                if (typeof metadata == "undefined") return;
                fontName = modelWrapper.localizedNameFromMetadata(metadata, familyCmb.locales, name);
                sample = modelWrapper.localizedSampleFromMetadata(metadata, familyCmb.locales, "");
            }

            /// When updating the model wrapper, the "model" doesn't always update on the delegate, so we need to manually load
            /// the metadata from the modelwrapper.
            width: ListView.view.width;
            highlighted: resourceView.highlightedIndex === model.index;
            property bool selected: resourceView.modelWrapper.currentIndex === model.index;

            palette: familyCmb.palette;

            contentItem: Control {
                id: delegateControl;

                property alias highlighted: fontDelegateItem.highlighted;
                palette: fontDelegateItem.palette;

                property alias fontName: fontDelegateItem.fontName;

                implicitHeight: nameLabel.height*4;

                Loader {
                    anchors.fill: parent;
                    asynchronous: true;

                    sourceComponent: Component {
                        KoShapeQtQuickLabel {
                            id: fontFamilyDelegate;

                            Binding {
                                   target: fontFamilyDelegate
                                   property: "meta"
                                   when: fontDelegateItem && typeof fontDelegateItem !== 'undefined' && typeof fontDelegateItem.metadata !== 'undefined';
                                   value: fontDelegateItem.metadata;
                            }

                            property var meta: ({});
                            property bool colorBitmap : typeof meta.color_bitmap === 'boolean'? meta.color_bitmap: false;
                            property bool colorCLRV0 : typeof meta.color_clrv0 === 'boolean'?  meta.color_clrv0: false;
                            property bool colorCLRV1 : typeof meta.color_clrv1 === 'boolean'?  meta.color_clrv1: false;
                            property bool colorSVG : typeof meta.color_svg === 'boolean'?  meta.color_svg: false;
                            property bool isVariable : typeof meta.is_variable === 'boolean'?  meta.is_variable: false;
                            property int type : typeof meta.font_type === 'number'? meta.font_type: 0;

                            scalingType: KoShapeQtQuickLabel.FitHeight;
                            svgData: fontDelegateItem.sample;
                            foregroundColor: delegateControl.highlighted? palette.highlightedText: palette.text;
                            fillColor: delegateControl.highlighted? familyCmb.palette.highlight: familyCmb.palette.window;
                            fullColor: colorBitmap || colorCLRV0 || colorCLRV1 || colorSVG;
                            padding: nameLabel.height;

                            Row {
                                id: featureRow;
                                spacing: columnSpacing;
                                anchors.bottom: parent.bottom;
                                anchors.left: parent.left;
                                property int imgHeight: 12;
                                ToolButton {
                                    enabled: false;
                                    padding:0;
                                    width: parent.imgHeight;
                                    height: parent.imgHeight;
                                    property int type: fontFamilyDelegate.type;
                                    icon.width: parent.imgHeight;
                                    icon.height: parent.imgHeight;
                                    palette: fontDelegateItem.palette;
                                    icon.color: nameLabel.color;
                                    icon.source: type === KoSvgText.BDFFontType? "qrc:///font-type-bitmap.svg"
                                                                          : type === KoSvgText.Type1FontType? "qrc:///font-type-postscript.svg"
                                                                                                            : type === KoSvgText.OpenTypeFontType? "qrc:///font-type-opentype.svg":"qrc:///light_system-help.svg";
                                }
                                ToolButton {
                                    enabled: false;
                                    padding: 0;
                                    width: parent.imgHeight;
                                    height: parent.imgHeight;
                                    icon.width: parent.imgHeight;
                                    icon.height: parent.imgHeight;
                                    palette: fontDelegateItem.palette;
                                    icon.color: nameLabel.color;
                                    visible: fontFamilyDelegate.isVariable;
                                    icon.source: "qrc:///font-type-opentype-variable.svg"
                                }

                                ToolButton {
                                    enabled: false;
                                    padding: 0;
                                    width: parent.imgHeight;
                                    height: parent.imgHeight;
                                    icon.width: parent.imgHeight;
                                    icon.height: parent.imgHeight;
                                    visible: fontFamilyDelegate.colorBitmap;
                                    icon.source: "qrc:///font-color-type-clr-bitmap.svg"
                                }
                                ToolButton {
                                    enabled: false
                                    padding: 0;
                                    width: parent.imgHeight;
                                    height: parent.imgHeight;
                                    icon.width: parent.imgHeight;
                                    icon.height: parent.imgHeight;
                                    visible: fontFamilyDelegate.colorCLRV0;
                                    icon.source: "qrc:///font-color-type-clr-v0.svg"
                                }
                                ToolButton {
                                    padding: 0;
                                    enabled: false;
                                    width: parent.imgHeight;
                                    height: parent.imgHeight;
                                    icon.width: parent.imgHeight;
                                    icon.height: parent.imgHeight;
                                    visible: fontFamilyDelegate.colorCLRV1;
                                    icon.source: "qrc:///font-color-type-clr-v1.svg"
                                }
                                ToolButton {
                                    enabled: false;
                                    padding: 0;
                                    width: parent.imgHeight;
                                    height: parent.imgHeight;
                                    icon.width: parent.imgHeight;
                                    icon.height: parent.imgHeight;
                                    visible: fontFamilyDelegate.colorSVG;
                                    icon.source: "qrc:///font-color-type-svg.svg"
                                }
                            }
                        }
                    }
                }

                Label {
                    id: nameLabel;
                    palette: fontDelegateItem.palette;
                    text: delegateControl.fontName;
                    anchors.top: parent.top;
                    anchors.left: parent.left;
                    elide: Text.ElideRight;
                    width: parent.width;
                    color: parent.highlighted? palette.highlightedText: palette.text;
                }

            }
            background: Rectangle {
                id: fontDelegateBackground
                color: highlighted? familyCmb.palette.highlight: familyCmb.palette.window;
            }

            MouseArea {
                id: delegateMouseArea;
                acceptedButtons: Qt.RightButton | Qt.LeftButton;
                anchors.fill: parent;
                hoverEnabled: true;
                // Itemdelegate can also handle hovers, but we don't use those.
                preventStealing: true;
                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        resourceView.openContextMenu(mouse.x, mouse.y, parent.model.name, parent.model.index);
                    } else {
                        resourceView.applyHighlightedIndex();
                        familyCmb.activated();
                    }
                }
                onContainsMouseChanged: {
                    if(containsMouse) {
                        resourceView.highlightedIndex = parent.index;
                    }
                }

                Kis.ToolTipBase {
                    text: fontDelegateItem.fontName;
                    visible: delegateMouseArea.containsMouse;
                }
            }
        }
    }

    //--- Pop up setup ---//
    Popup {
        id: familyCmbPopup;
        y: familyCmb.height - 1;
        x: familyCmb.width - width;
        width: Math.max(contentWidth, Math.max(familyCmb.width, 200));
        height: Math.min(contentItem.implicitHeight, familyCmb.maxPopupHeight - topMargin - bottomMargin)
        padding: 2;

        palette: familyCmb.palette;

        contentItem: ResourceView {
            id: resourceView;
            resourceType: "fontfamilies";
            resourceDelegate: fontDelegate;
            palette: familyCmbPopup.palette;
            addResourceRow.visible: false;
        }
    }
}
