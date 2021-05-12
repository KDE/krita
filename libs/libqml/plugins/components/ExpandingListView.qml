/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base
    property alias model: listView.model;
    property alias currentIndex: listView.currentIndex;
    signal newIndex(int index);
    height: topButton.height;

    Rectangle {
        id: topButton
        border {
            width: 2;
            color: Settings.theme.color("components/expandingListView/selection/border");
        }
        radius: height * 0.5;
        anchors {
            top: base.expandToTop ? undefined : parent.top;
            bottom: base.expandToTop ? parent.bottom : undefined;
            right: parent.right;
            left: parent.left;
        }
        width: (Constants.GridWidth * 2) - 8 ;
        height: Constants.GridHeight * 0.75;
        color: Settings.theme.color("components/expandingListView/selection/fill");
        z: 10;


        Image {
            id: arrowsList
            anchors {
                right: parent.right;
                verticalCenter: parent.verticalCenter;
                rightMargin: -1;
            }
            height: parent.height;
            width: height;
            sourceSize.height: height
            source: Settings.theme.icon("combo-arrows-white");
            smooth: true
        }

        Label {
            id: buttonText;
            anchors {
                verticalCenter: parent.verticalCenter;
                left: parent.left;
                leftMargin: parent.radius;
                right: arrowsList.left;
            }
            text: listView.currentItem ? listView.currentItem.text : "(tap to select)";
            color: Settings.theme.color("components/expandingListView/selection/text");
        }

        MouseArea {
            anchors.fill: parent;
            onClicked: {
                if (base.state === "expanded") {
                    base.state = "";
                }
                else {
                    base.state = "expanded";
                }
            }
        }
    }

    Rectangle {
        id: listContainer
        anchors {
            top: base.expandToTop ? parent.top : topButton.bottom;
            left: parent.left;
            right: parent.right;
            bottom: base.expandToTop ? topButton.top : parent.bottom;
            leftMargin: topButton.radius;
            rightMargin: topButton.radius;
        }
        clip: true;
        opacity: 0;
        color: Settings.theme.color("components/expandingListView/list/background");
        ListView {
            id: listView;
            anchors.fill: parent;
            delegate: Item {
                property alias text: delegateLabel.text
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
                height: Constants.DefaultFontSize + Constants.DefaultMargin * 2;
                Rectangle {
                    anchors.fill: parent;
                    radius: height / 2;
                    border.width: 2;
                    border.color: Settings.theme.color("components/expandingListView/list/itemBorder");
                    color: Settings.theme.color("components/expandingListView/list/item");
                    opacity: model.isCategory ? 0.3 : 0;
                }
                Shadow {
                    anchors {
                        bottom: delegateLabel.bottom;
                        left: delegateLabel.left;
                        right: delegateLabel.right;
                    }
                    visible: listView.currentIndex === index;
                }
                Label {
                    id: delegateLabel
                    anchors.fill: parent
                    anchors.leftMargin: model.isCategory ? Constants.DefaultFontSize / 2 : 0;
                    text: model.text ? model.text : index + " (no text field in model)";
                    color: Settings.theme.color("components/expandingListView/list/itemText");
                }
                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        if (model.isCategory) {
                            return;
                        }
                        listView.currentIndex = index;
                        base.newIndex(index);
                        base.state = "";
                    }
                }
            }
            ScrollDecorator {
                flickableItem: parent;
            }
        }
    }

    property bool expandToTop: false;
    property int expandedHeight: base.parent.height - base.y - (Constants.GridHeight / 2);
    states: [
        State {
            name: "expanded";
            PropertyChanges {
                target: base;
                height: expandedHeight > 0 ? expandedHeight : Constants.GridHeight * 2;
            }
            PropertyChanges {
                target: listContainer;
                opacity: 1;
            }
        }
    ]
    transitions: [
        Transition {
            from: ""
            to: "expanded"
            reversible: true;
            PropertyAnimation { properties: "height,opacity"; duration: Constants.AnimationDuration; easing.type: Easing.InOutCubic }
        }
    ]
}
