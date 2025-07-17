/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15

Item {
    id: root

    property bool showTopBar: false
    property bool topBarActive: false
    property real radius: 0.0
    property color color: "blue"
    property real topBarWidth: 0.0
    property real bottomBarWidth: 0.0
    property bool faded: false
    property real exponentRatio: 1.0

    states: [
        State {
            name: "faded"
            when: root.faded

            PropertyChanges {
                target: root;
                    opacity: 0.5
                }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"

            PropertyAnimation {
                target: root
                property: "opacity"
                duration: 200
            }
        }
    ]

    Item {
        anchors.fill: parent

        states: [
            State {
                name: "topBarShown"
                when: root.showTopBar && !root.topBarActive

                PropertyChanges {
                    target: bottomBarContainer;
                    height: root.height - 3
                }
            },
            State {
                name: "topBarShownAndActive"
                when: root.showTopBar && root.topBarActive

                PropertyChanges {
                    target: bottomBarContainer;
                    height: 3
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "*"
                PropertyAnimation {
                    target: bottomBarContainer
                    property: "height"
                    duration: 150
                }
            }
        ]

        Item {
            id: topBarContainer
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: bottomBarContainer.top
            width: Math.max(0, Math.min(1, Math.pow(root.topBarWidth, 1 / root.exponentRatio))) * root.width
            clip: true

            Rectangle {
                id: topBar
                width: topBarContainer.width + root.radius
                height: Math.max(root.radius * 2, topBarContainer.height + root.radius)
                opacity: 0.75
                color: root.color
                radius: root.radius
            }
        }

        Item {
            id: bottomBarContainer
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: Math.max(0, Math.min(1, Math.pow(root.bottomBarWidth, 1 / root.exponentRatio))) * root.width
            height: root.height
            clip: true

            Rectangle {
                id: bottomBar
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                width: bottomBarContainer.width + root.radius
                height: root.showTopBar ? Math.max(root.radius * 2, bottomBarContainer.height + root.radius) : bottomBarContainer.height
                color: root.color
                radius: root.radius
            }
        }
    }
}
