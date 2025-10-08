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

/**
  \qmltype ResourceView
  This provides a resource selector complete with filters, tagging and more.
  */
Control {
    id: control;
    property Kis.TagFilterProxyModelQmlWrapper modelWrapper: Kis.TagFilterProxyModelQmlWrapper {
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
        \qmlProperty highlightedIndex
        The currently hovered index on the resource selector.
     */
    property alias highlightedIndex : view.currentIndex;

    /*
        \qmlProperty resourceDelegate
        Resource Delegate. This is used to draw the delegates.
     */
    property alias resourceDelegate : view.delegate;

    /*
        \qmlProperty addResourceRowVisible
        Whether the resource row should be visible.
     */
    property alias addResourceRowVisible: addResourceRow.visible;
    /*
        \qmlProperty resourceDelegate
        The add resource row, this contains by default import and remove resource.
     */
    property alias addResourceRow: addResourceRow;
    /*
        \qmlProperty additionalAddResourceRow
        The additional resource row can be used to add extra toolbuttons.
     */
    property alias additionalAddResourceRow: addResourceRow.data;

    /*
        \qmlProperty showTagging
        Show tagging functionality, bool.
     */
    property alias showTagging: tagAndConfig.visible;
    /*
        \qmlProperty showSearch
        Show the search bar, bool.
     */
    property alias showSearch: searchBar.visible;

    /*
      \qmlProperty viewPreferredHeight
        Preferred height of the resource list view.
     */
    property int preferredHeight: font.pixelSize * 15;
    /*
      \qmlProperty viewMinimumHeight
        Minimum height of the resource list view.
     */
    property int minimumHeight: font.pixelSize * 3;

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
                        tagActionsContextMenu.resourceIndex = modelWrapper.currentIndex;
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
            id:viewFrame;
            Layout.minimumHeight: control.minimumHeight + bottomPadding + topPadding;
            Layout.fillWidth: true;
            Layout.fillHeight: true;
            Layout.preferredHeight: control.preferredHeight + bottomPadding + topPadding;
            Layout.maximumHeight: control.maximumHeight - (bottomPadding + topPadding);
            palette: control.palette;
            padding: 1;
            bottomPadding: padding;
            topPadding: padding;

            background: Rectangle {
                border.color: palette.mid;
                border.width: viewFrame.padding;
                color: "transparent";
            }

            ListView {
                clip: true;
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
                icon.source: "qrc:///light_document-import-16.svg";
                icon.width: 16;
                icon.color: palette.buttonText;
                hoverEnabled: true;

                display: AbstractButton.IconOnly;
                text: i18nc("@info:tooltip, %1 is resource type name", "Import %1", modelWrapper.resourceTypeName);
                enabled: modelWrapper.importEnabled;

                Kis.ThemedControl {
                    id: importPal;
                }
                palette: importPal.palette;

                Kis.ToolTipBase {
                    text: importResource.text;
                    visible: importResource.hovered;
                }

                onClicked: modelWrapper.importResource();

            }
            ToolButton {
                id: removeResource;
                palette: control.palette;
                icon.source: "qrc:///light_edit-delete.svg";
                icon.width: 16;
                icon.color: palette.text;
                hoverEnabled: true;
                display: AbstractButton.IconOnly;
                text: i18nc("@info:tooltip, %1 is resource type name", "Remove %1", modelWrapper.resourceTypeName);

                Kis.ToolTipBase {
                    text: removeResource.text;
                    visible: removeResource.hovered;
                }

                onClicked: modelWrapper.removeResource();
            }
        }
    }
}
