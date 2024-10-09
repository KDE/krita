/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0

Button {
    id: familyCmb;
    wheelEnabled: true;

    //--- Model setup. ---//

    property TagFilterProxyModelQmlWrapper modelWrapper : TagFilterProxyModelQmlWrapper{
        id: modelWrapperId;
    };
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
            /// When updating the model wrapper, the "model" doesn't always update on the delegate, so we need to manually load
            /// the metadata from the modelwrapper.
            property var meta: familyCmb.modelWrapper.metadataForIndex(model.index);
            width: fontResourceView.listWidth;
            highlighted: familyCmb.highlightedIndex === model.index;

            Component.onCompleted: {
                fontName = familyCmb.modelWrapper.localizedNameForIndex(model.index, locales, model.name);
            }

            contentItem: KoShapeQtQuickLabel {
                id: fontFamilyDelegate;
                property alias meta: fontDelegateItem.meta;
                property bool colorBitmap : meta.color_bitmap;
                property bool colorCLRV0 : meta.color_clrv0;
                property bool colorCLRV1 : meta.color_clrv1;
                property bool colorSVG : meta.color_svg;
                property bool isVariable : meta.is_variable;
                property int type : meta.font_type;
                property string fontName: fontDelegateItem.fontName;
                width: parent.width;
                height: nameLabel.height * 5;
                imageScale: 3;
                imagePadding: nameLabel.height;
                svgData: meta.sample_svg;
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
                acceptedButtons: Qt.RightButton | Qt.LeftButton;
                anchors.fill: parent;
                hoverEnabled: true;
                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        openContextMenu(mouse.x, mouse.y);
                    } else {
                        familyCmb.modelWrapper.currentIndex = fontDelegateItem.model.index;
                        familyCmb.activated();
                    }
                }
                onHoveredChanged: familyCmb.highlightedIndex = fontDelegateItem.model.index;

                function openContextMenu(x, y) {
                    tagActionsContextMenu.resourceIndex = fontDelegateItem.model.index;
                    tagActionsContextMenu.popup()
                }

            }
        }
    }

    //--- Pop up setup ---//
    Popup {
        id: familyCmbPopup;
        y: familyCmb.height - 1;
        x: familyCmb.width - width;
        width: contentWidth;
        height: contentHeight;
        padding: 2;

        contentItem:
            ColumnLayout {
            id: fontResourceView;
            clip:true;
            property alias fontModel : view.model;
            property alias tagModel: tagFilter.model;
            property alias listWidth : view.width;
            fontModel: familyCmb.modelWrapper.model;
            tagModel: familyCmb.modelWrapper.tagModel;

            RowLayout {
                id: tagAndConfig;

                ComboBox {
                    id: tagFilter;
                    textRole: "display";
                    Layout.fillWidth: true;
                    currentIndex: familyCmb.modelWrapper.currentTag;
                    onActivated: familyCmb.modelWrapper.currentTag = currentIndex;
                }

                Button {
                    id: tagMenuButton;
                    icon.source: "qrc:///light_bookmarks.svg";
                    text: i18nc("@label:button", "Tag");
                    onClicked: hideShowMenu();
                    function hideShowMenu() {
                        if (!tagActionsContextMenu.visible) {
                            tagActionsContextMenu.resourceIndex = familyCmb.highlightedIndex;
                            tagActionsContextMenu.popup(tagMenuButton,
                                                        tagActionsContextMenu.width - tagMenuButton.width,
                                                        tagMenuButton.height - 1);
                        } else {
                            tagActionsContextMenu.dismiss();
                        }
                    }

                    //--- Tag Setup ---//
                    Menu {
                        id: tagActionsContextMenu;
                        property int resourceIndex: -1;
                        property alias resourceName: resourceLabel.text;
                        property var resourceTaggedModel : [];
                        enabled: resourceIndex >= 0;
                        onResourceIndexChanged: {
                            resourceName = modelWrapper.localizedNameForIndex(resourceIndex, locales);
                            updateResourceTaggedModel();
                        }
                        function updateResourceTaggedModel() {
                            resourceTaggedModel = modelWrapper.taggedResourceModel(tagActionsContextMenu.resourceIndex);
                        }
                        function updateAndDismiss() {
                            updateResourceTaggedModel();
                            dismiss();
                        }

                        modal: true;
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                        Label {
                            id: resourceLabel;
                        }

                        Menu {
                            id: assignToTag;
                            title: i18nc("@title:menu", "Assign to Tag");
                            height: contentChildren.height;
                            ListView {
                                id: tagAddView;
                                model: tagActionsContextMenu.resourceTaggedModel;
                                height: contentHeight;
                                width: parent.width;

                                delegate: ItemDelegate {
                                    width: tagAddView.width;
                                    text: modelData.name;
                                    visible: modelData.visible && !modelData.enabled;
                                    height: visible? implicitContentHeight: 0;

                                    onClicked: {
                                        modelWrapper.tagResource(modelData.value, tagActionsContextMenu.resourceIndex);
                                        tagActionsContextMenu.updateAndDismiss();
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
                                        newTagName.text = "";
                                        tagActionsContextMenu.updateAndDismiss();
                                    }
                                }
                            }

                        }
                        MenuSeparator {}
                        Action {
                            enabled: modelWrapper.showResourceTagged(modelWrapper.currentTag, tagActionsContextMenu.resourceIndex);
                            id: removeFromThisTag;
                            text: i18nc("@action:inmenu", "Remove from this tag");
                            icon.source: "qrc:///16_light_list-remove.svg";
                            onTriggered: modelWrapper.untagResource(modelWrapper.currentTag, tagActionsContextMenu.resourceIndex);
                        }

                        Menu {
                            id: removeFromTag;
                            title: i18nc("@title:menu", "Remove from other tag");
                            ListView {
                                id: tagRemoveView;
                                model: tagActionsContextMenu.resourceTaggedModel;
                                height: contentHeight;
                                width: parent.width;
                                delegate: ItemDelegate {
                                    width: tagRemoveView.width;
                                    text: modelData.name;
                                    visible: modelData.visible && modelData.enabled;
                                    height: visible? implicitContentHeight: 0;
                                    onClicked: {
                                        modelWrapper.untagResource(modelData.value, tagActionsContextMenu.resourceIndex);
                                        tagActionsContextMenu.updateAndDismiss()
                                    }
                                }
                            }
                        }
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
                    currentIndex: familyCmb.modelWrapper.currentIndex;
                    delegate: fontDelegate;
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
        }
        palette: familyCmb.palette;

    }
}
