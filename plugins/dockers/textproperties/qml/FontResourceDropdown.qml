/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

ComboBox {
    id: familyCmb;

    Component {
        id: fontDelegate
        ItemDelegate {
            required property var model;

            highlighted: familyCmb.highlightedIndex === model.index;
            contentItem: KoShapeQtQuickLabel{
                id: fontFamilyDelegate;
                property bool colorBitmap : model.metadata.color_bitmap;
                property bool colorCLRV0 : model.metadata.color_clrv0;
                property bool colorCLRV1 : model.metadata.color_clrv1;
                property bool colorSVG : model.metadata.color_svg;
                property bool isVariable : model.metadata.is_variable;
                property int type : model.metadata.font_type;
                property string fontName: model.name;

                Layout.fillWidth: true;
                svgData: model.metadata.sample_svg;
                imageScale: 3;
                imagePadding: nameLabel.height;
                foregroundColor: sysPalette.text;
                fullColor: colorBitmap || colorCLRV0 || colorCLRV1 || colorSVG;

                Component.onCompleted: {
                    var localizedDict = model.metadata.localized_font_family;
                    for (var i in locales) {
                        var key = locales[i];
                        if (key in localizedDict) {
                            fontName = localizedDict[key];
                            break;
                        }
                    }
                }

                Label {
                    id: nameLabel;
                    text: fontFamilyDelegate.fontName;
                    anchors.top: parent.top;
                    anchors.left: parent.left;
                }

                Row {
                    id: featureRow;
                    spacing: columnSpacing;
                    anchors.bottom: parent.bottom;
                    anchors.left: parent.left;
                    property int imgHeight: 12;
                    Image {
                        property int type: fontFamilyDelegate.type;
                        width: parent.imgHeight;
                        height: parent.imgHeight;
                        source: type === KoSvgText.BDFFontType? "qrc:///light_select-pixel.svg"
                                                              : type === KoSvgText.Type1FontType? "qrc:///light_transparency-enabled.svg"
                                                                                                : type === KoSvgText.OpenTypeFontType? "qrc:///light_select-shape.svg":"qrc:///light_system-help.svg";
                    }
                    Image {
                        width: parent.imgHeight;
                        height: parent.imgHeight;
                        visible: fontFamilyDelegate.isVariable;
                        source: "qrc:///light_zoom-horizontal.svg"
                    }

                    Image {
                        width: parent.imgHeight;
                        height: parent.imgHeight;
                        visible: fontFamilyDelegate.colorBitmap;
                        source: "qrc:///light_palette-library.svg"
                    }
                    Image {
                        width: parent.imgHeight;
                        height: parent.imgHeight;
                        visible: fontFamilyDelegate.colorCLRV0;
                        source: "qrc:///light_color-adjustment-mode-channels.svg"
                    }
                    Image {
                        width: parent.imgHeight;
                        height: parent.imgHeight;
                        visible: fontFamilyDelegate.colorCLRV1;
                        source: "qrc:///light_config-color-manage.svg"
                    }
                    Image {
                        width: parent.imgHeight;
                        height: parent.imgHeight;
                        visible: fontFamilyDelegate.colorSVG;
                        source: "qrc:///light_sort-by-hue.svg"
                    }
                }
            }
            width: fontResourceView.listWidth;
            background: Rectangle { color: highlighted? parent.palette.highlight:"transparent"; }
        }
    }


    textRole: "name";

    delegate: fontDelegate;
    popup: Popup {
        y: familyCmb.height - 1;
        x: familyCmb.width - width;
        width: contentWidth;
        height: contentHeight;
        padding: 2;

        contentItem:
            ColumnLayout {
            id: fontResourceView;
            fontModel: familyCmb.delegateModel;
            tagModel: fontTagModel;
            currentIndex: familyCmb.highlightedIndex;
            clip:true;
            property var fontModel;
            property var tagModel;
            property alias currentIndex : view.currentIndex;
            property alias listWidth : view.width;

            RowLayout {
                id: tagAndConfig;

                ComboBox {
                    id: tagFilter;
                    model: fontResourceView.tagModel;
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
                    model: fontResourceView.fontModel;
                    ScrollBar.vertical: ScrollBar {
                    }
                }
            }
            RowLayout {
                Layout.preferredHeight: childrenRect.height;
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
        palette: familyCmb.palette;
    }
    wheelEnabled: true;
}
