/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.krita.flake.text 1.0
import org.krita.components 1.0 as Kis

Control {
    id: control;
    property TagFilterProxyModelQmlWrapper modelWrapper: TagFilterProxyModelQmlWrapper {
        resourceType: control.resourceType;
    };

    Connections {
        target: modelWrapper;
        function onCurrentIndexChanged() {
            control.highlightedIndex = modelWrapper.currentIndex;
        }
        function onResourceTypeChanged() {
            control.resourceType = modelWrapper.resourceType;
        }
    }

    /*
        \qmlProperty resourceType
        A string corresponding to KisResourceTypes.h ResourceType.
        Used to set the resource model and tagging models.

        Because modelwrapper can change, we cannot use aliasing here.
      */
    property string resourceType;
    onResourceTypeChanged: modelWrapper.resourceType = resourceType;


    /*

     */
    property alias highlightedIndex : view.currentIndex;

    property alias resourceDelegate : view.delegate;

    property alias addResourceRow: addResourceRow;
    property alias additionalAddResourceRow: addResourceRow.data;

    property alias showTagging: tagAndConfig.visible;
    property alias showSearch: searchBar.visible;

    function downPress() {
        highlightedIndex += 1;
    }
    function upPress() {
        highlightedIndex -= 1;
    }

    function applyHighlightedIndex() {
        if (modelWrapper) {
            modelWrapper.currentIndex = highlightedIndex;
        }
    }

    Keys.onDownPressed: downPress();
    Keys.onUpPressed: upPress();

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
            height: visible? implicitHeight: 0;

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
                        palette: control.palette;
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
                        palette: control.palette;
                        onTriggered: modelWrapper.untagResource(modelWrapper.currentTag, tagActionsContextMenu.resourceIndex);
                    }

                    Menu {
                        id: removeFromTag;
                        title: i18nc("@title:menu", "Remove from other tag");
                        palette: control.palette;

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
            Layout.preferredHeight: font.pixelSize*15;
            palette: control.palette;
            clip: true;
            ListView {
                anchors.fill: parent;
                id: view;
                currentIndex: 0;

                Keys.onDownPressed: control.downPress();
                Keys.onUpPressed: control.upPress();

                ScrollBar.vertical: ScrollBar {
                }
            }
        }
        RowLayout {
            id: searchBar;
            height: visible? implicitHeight: 0;
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
