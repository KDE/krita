/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.flake.text 1.0

Control {
    id: control;
    property TagFilterProxyModelQmlWrapper modelWrapper: TagFilterProxyModelQmlWrapper{
        id: modelWrapperId;
    };
    property alias resourceType : modelWrapperId.resourceType;
    property alias currentIndex : modelWrapperId.currentIndex;
    property alias currentResource : modelWrapperId.currentResource;

    property alias resourceDelegate : view.delegate;

    property alias addResourceRow: addResourceRow;
    property alias additionalAddResourceRow: addResourceRow.data;

    function openContextMenu(x, y, resourceName, resourceIndex) {
        tagActionsContextMenu.resourceName = resourceName;
        tagActionsContextMenu.resourceIndex = resourceIndex;
        tagActionsContextMenu.popup(x, y);
    }

    contentItem: ColumnLayout {
        id: resourceView;
        clip:true;
        anchors.fill: parent;
        property alias resourceModel : view.model;
        property alias tagModel: tagFilter.model;
        resourceModel: modelWrapper.model;
        tagModel: modelWrapper.tagModel;

        RowLayout {
            id: tagAndConfig;

            ComboBox {
                id: tagFilter;
                textRole: "display";
                Layout.fillWidth: true;
                currentIndex: modelWrapper.currentTag;
                onActivated: modelWrapper.currentTag = currentIndex;
                palette: control.palette;
            }

            Button {
                id: tagMenuButton;
                icon.source: "qrc:///light_bookmarks.svg";
                icon.color: palette.text;
                text: i18nc("@label:button", "Tag");
                onClicked: hideShowMenu();
                palette: control.palette;
                function hideShowMenu() {
                    if (!tagActionsContextMenu.visible) {
                        tagActionsContextMenu.resourceIndex = control.highlightedIndex;
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

                    palette: control.palette;
                    property int resourceIndex: -1;
                    property alias resourceName: resourceLabel.text;
                    property var resourceTaggedModel : [];
                    enabled: resourceIndex >= 0;
                    onResourceIndexChanged: {
                        resourceName = modelWrapper.localizedNameFromMetadata(modelWrapper.metadataForIndex(tagActionsContextMenu.resourceIndex), locales, resourceName);
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
                        padding: parent.padding;
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
                    MenuItem {
                        enabled: modelWrapper.showResourceTagged(modelWrapper.currentTag, tagActionsContextMenu.resourceIndex);
                        id: removeFromThisTag;
                        text: i18nc("@action:inmenu", "Remove from this tag");
                        icon.source: "qrc:///16_light_list-remove.svg";
                        icon.color: palette.text;
                        PaletteControl {
                            id: pal;
                            colorGroup: parent.enabled? SystemPalette.Active: SystemPalette.Disabled;
                        }
                        palette: pal.palette;
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
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            Layout.preferredHeight: contentHeight;
            palette: control.palette;
            clip: true;
            ListView {
                anchors.fill: parent;
                id: view;
                implicitHeight: contentHeight;
                currentIndex: modelWrapper.currentIndex;
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
                palette: control.palette;
            }
            CheckBox {
                id: searchInTagCbx
                text: i18nc("@option:check", "Search in tag")
                onCheckedChanged: modelWrapper.searchInTag = checked;
                checked: modelWrapper.searchInTag;
                palette: control.palette;
            }
        }
        RowLayout {
            id: addResourceRow;

            height: visible? implicitHeight: 0;

            ToolButton {
                id: importResource;
                palette: control.palette;
                icon.source: "qrc:///light_document-import-16.svg";
                icon.width: 16;
                icon.color: palette.text;
                hoverEnabled: true;
                ToolTip.text: i18nc("@info:tooltip", "Import Resource");
                display: AbstractButton.IconOnly;
                text: ToolTip.text;
                ToolTip.visible: hovered;
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;

                onClicked: modelWrapper.importResource();

            }
            ToolButton {
                id: removeResource;
                palette: control.palette;
                icon.source: "qrc:///light_edit-delete.svg";
                icon.width: 16;
                icon.color: palette.text;
                hoverEnabled: true;
                ToolTip.text: i18nc("@info:tooltip", "Remove Resource");
                display: AbstractButton.IconOnly;
                text: ToolTip.text;
                ToolTip.visible: hovered;
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval;

                onClicked: modelWrapper.removeResource();
            }
        }
    }
}
