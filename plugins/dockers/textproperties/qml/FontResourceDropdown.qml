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
    wheelEnabled: true;

    //--- Model setup. ---//
    property var sourceModel;
    property TagFilterProxyModelQmlWrapper modelWrapper : TagFilterProxyModelQmlWrapper{
        sourceModel: sourceModel;
    };
    model: modelWrapper.model;
    textRole: "name";

    //--- Delegate setup. ---//
    Component {
        id: fontDelegate
        ItemDelegate {
            id: fontDelegateItem;
            required property var model;
            property string fontName: model.name;
            width: fontResourceView.listWidth;
            highlighted: familyCmb.highlightedIndex === model.index;

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

            contentItem: KoShapeQtQuickLabel {
                id: fontFamilyDelegate;
                property bool colorBitmap : model.metadata.color_bitmap;
                property bool colorCLRV0 : model.metadata.color_clrv0;
                property bool colorCLRV1 : model.metadata.color_clrv1;
                property bool colorSVG : model.metadata.color_svg;
                property bool isVariable : model.metadata.is_variable;
                property int type : model.metadata.font_type;
                property string fontName: fontDelegateItem.fontName;
                width: parent.width;
                height: nameLabel.height * 5;
                imageScale: 3;
                imagePadding: nameLabel.height;
                svgData: model.metadata.sample_svg;
                foregroundColor: sysPalette.text;
                fullColor: colorBitmap || colorCLRV0 || colorCLRV1 || colorSVG;

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
            background: Rectangle {
                color: highlighted? familyCmb.palette.highlight:"transparent";
            }

            MouseArea {
                acceptedButtons: Qt.RightButton;
                anchors.fill: parent;
                //preventStealing: true;
                onClicked: openContextMenu(mouse.x, mouse.y);
                //onPressAndHold: openContextMenu(mouse.x, mouse.y);

                function openContextMenu(x, y) {
                    tagActionsContextMenu.resourceName = fontDelegateItem.fontName;
                    tagActionsContextMenu.resourceIndex = model.index;
                    tagActionsContextMenu.x = x;
                    tagActionsContextMenu.y = y;
                    tagActionsContextMenu.visible = true;
                }
            }
        }
    }
    delegate: fontDelegate;

    //--- Pop up setup ---//
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
            currentIndex: familyCmb.currentIndex;
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
                    currentIndex: modelWrapper.currentTag;
                    onActivated: modelWrapper.currentTag = currentIndex;
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
                    text: modelWrapper.searchText;
                    onTextEdited: modelWrapper.searchText = text;
                }
                CheckBox {
                    id: opticalSizeCbx
                    text: i18nc("@option:check", "Search in tag")
                    onCheckedChanged: modelWrapper.searchInTag = checked;
                    checked: modelWrapper.searchInTag;
                }
            }

            //--- Tag Setup ---//
            Menu {
                id: tagActionsContextMenu;
                property int resourceIndex: -1;
                property string resourceName: "";
                Label {
                    id: resourceLabel;
                    text: tagActionsContextMenu.resourceName;
                }

                Menu {
                    id: assignToTag;
                    title: i18nc("@title:menu", "Assign to Tag");
                    // submenus can't have icons in qml...
                    //icon: "qrc:///light_list-add.svg";
                    height: contentChildren.height;
                    ListView {
                        id: tagAddView;
                        model: fontTagModel;
                        height: contentHeight;
                        width: parent.width;
                        delegate: ItemDelegate {
                            width: tagAddView.width;
                            text: model.display;
                            onClicked: {
                                modelWrapper.tagResource(model.index, tagActionsContextMenu.resourceIndex);
                                tagActionsContextMenu.close();
                            }
                        }
                    }
                    MenuSeparator {}
                    RowLayout {
                        TextField {
                            id: newTagName;
                            placeholderText: i18nc("@info:placeholder", "New Tag Name...");
                            Layout.fillWidth: true;
                        }
                        ToolButton {
                            icon.source: "qrc:///light_list-add.svg";
                            icon.color: palette.text;
                            onClicked:  {
                                modelWrapper.addNewTag(newTagName.text, tagActionsContextMenu.resourceIndex);
                                tagActionsContextMenu.visible = false;
                            }
                        }
                    }

                }
                MenuSeparator {}
                Action {
                    enabled: tagFilter.currentIndex != 0;
                    id: removeFromThisTag;
                    text: i18nc("@action:inmenu", "Remove from this tag");
                    icon.source: "qrc:///16_light_list-remove.svg";
                }

                Menu {
                    id: removeFromTag;
                    title: i18nc("@title:menu", "Remove from other tag");
                    ListView {
                        id: tagRemoveView;
                        model: fontTagModel;
                        height: contentHeight;
                        width: parent.width;
                        delegate: ItemDelegate {
                            width: tagRemoveView.width;
                            text: model.display;
                            onClicked: {
                                modelWrapper.untagResource(model.index, tagActionsContextMenu.resourceIndex);
                                tagActionsContextMenu.visible = false;
                            }
                        }
                    }
                }
            }
        }
        palette: familyCmb.palette;


    }


}
