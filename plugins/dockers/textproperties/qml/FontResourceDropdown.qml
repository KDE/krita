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

Button {
    id: familyCmb;
    wheelEnabled: true;

    //--- Model setup. ---//

    property TagFilterProxyModelQmlWrapper modelWrapper : TagFilterProxyModelQmlWrapper{
        id: modelWrapperId;
        resourceType: "fontfamilies";
    };

    /// Ideally we'd have the max popup height be Window-height - y-pos-of-item-to-window.
    /// But that's pretty hard to calculate (map to global is screen relative, but there's
    /// no way to get window.y relative to screen), so we'll just use 300 as the maximum.
    property int maxPopupHeight: Math.min(Window.height, 300) - height*3;

    text: modelWrapper.resourceFilename;
    property int highlightedIndex;
    onClicked: {
        if (familyCmbPopup.visible) {
            familyCmbPopup.close();
        } else {
            familyCmbPopup.open();
        }
    }
    signal activated();
    onActivated: {
        familyCmbPopup.close();
    }

    //--- Delegate setup. ---//
    Component {
        id: fontDelegate
        ItemDelegate {
            id: fontDelegateItem;
            required property var model;
            property string fontName: model.name;
            property var meta: familyCmb.modelWrapper.metadataForIndex(model.index);
            // TODO: change this to use the text locale, if possible.
            property string sample: familyCmb.modelWrapper.localizedSampleFromMetadata(meta, locales, "");
            /// When updating the model wrapper, the "model" doesn't always update on the delegate, so we need to manually load
            /// the metadata from the modelwrapper.
            width: ListView.view.width;
            highlighted: familyCmb.highlightedIndex === model.index;

            Component.onCompleted: {
                fontName = familyCmb.modelWrapper.localizedNameFromMetadata(meta, locales, model.name);
            }

            palette: familyCmb.palette;

            contentItem: KoShapeQtQuickLabel {
                id: fontFamilyDelegate;
                property alias meta: fontDelegateItem.meta;
                property alias highlighted: fontDelegateItem.highlighted;
                property alias palette: fontDelegateItem.palette;
                property bool colorBitmap : typeof meta.color_bitmap === 'boolean'? meta.color_bitmap: false;
                property bool colorCLRV0 : typeof meta.color_clrv0 === 'boolean'?  meta.color_clrv0: false;
                property bool colorCLRV1 : typeof meta.color_clrv1 === 'boolean'?  meta.color_clrv1: false;
                property bool colorSVG : typeof meta.color_svg === 'boolean'?  meta.color_svg: false;
                property bool isVariable : typeof meta.is_variable === 'boolean'?  meta.is_variable: false;
                property int type : typeof meta.font_type === 'number'? meta.font_type: 0;
                property string fontName: fontDelegateItem.fontName;
                padding: nameLabel.height;
                scalingType: KoShapeQtQuickLabel.FitHeight;
                svgData: fontDelegateItem.sample;
                foregroundColor: highlighted? palette.highlightedText: palette.text;
                fullColor: colorBitmap || colorCLRV0 || colorCLRV1 || colorSVG;
                implicitHeight: padding*4;

                Label {
                    id: nameLabel;
                    palette: fontDelegateItem.palette;
                    text: fontFamilyDelegate.fontName;
                    anchors.top: parent.top;
                    anchors.left: parent.left;
                    elide: Text.ElideRight;
                    width: parent.width;
                    color: parent.highlighted? palette.highlightedText: palette.text;
                }

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
            background: Rectangle {
                color: highlighted? familyCmb.palette.highlight: "transparent";
            }

            MouseArea {
                acceptedButtons: Qt.RightButton | Qt.LeftButton;
                anchors.fill: parent;
                hoverEnabled: true;
                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        resourceView.openContextMenu(mouse.x, mouse.y, parent.model.name, parent.model.index);
                    } else {
                        familyCmb.modelWrapper.currentIndex = parent.model.index;
                        familyCmb.activated();
                    }
                }
                onHoveredChanged: familyCmb.highlightedIndex = parent.model.index;

                ToolTip.text: fontDelegateItem.fontName;
                ToolTip.visible: containsMouse;
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;

            }
        }
    }

    //--- Pop up setup ---//
    Popup {
        id: familyCmbPopup;
        y: familyCmb.height - 1;
        x: familyCmb.width - width;
        width: contentWidth;
        height: Math.min(contentItem.implicitHeight, familyCmb.maxPopupHeight - topMargin - bottomMargin)
        padding: 2;

        palette: familyCmb.palette;

        contentItem: ResourceView {
            id: resourceView;
            modelWrapper: familyCmb.modelWrapper;
            resourceDelegate: fontDelegate;
            palette: familyCmbPopup.palette;
        }
    }
}
