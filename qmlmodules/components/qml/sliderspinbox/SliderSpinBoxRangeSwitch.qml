/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import org.krita.components 1.0

/*
    \qmltype SliderSpinBoxRangeSwitch
    This item is a button to that can be used to switch between the hard and
    soft range that the slider spinboxes are capable of having these.

    It uses states to witch between hovered/enabled/not enabled to allow
    graceful transition between these states.
 */
Item {
    id: root

    /*
        \qmlproperty softRangeActive
        Boolean indicating whether the softrange is active now.
    */
    property bool softRangeActive: false
    /*
        \qmlproperty color
        The color of the button
        By default black, but should be set to palette text.
    */
    property color color: "black"
    /*
        \qmlproperty toolTip.
        The text of the tool tip.
    */
    property alias toolTip: toolTipPopup.text

    opacity: 0.4

    states: [
        State {
            name: "hovered"
            when: root.enabled && !root.softRangeActive && mouseArea.containsMouse

            PropertyChanges {
                target: root;
                opacity: 1
            }
        },
        State {
            name: "softRangeActive"
            when: root.softRangeActive && (!root.enabled || !mouseArea.containsMouse)

            PropertyChanges {
                target: rangeSwitchOuterCircle;
                opacity: 1
            }
            PropertyChanges {
                target: rangeSwitchInnerCircle;
                width: rangeSwitchOuterCircle.width * 0.5 - 1;
            }
        },
        State {
            name: "softRangeActiveAndHovered"
            when: root.enabled && root.softRangeActive && mouseArea.containsMouse

            PropertyChanges {
                target: rangeSwitchOuterCircle;
                opacity: 1
            }
            PropertyChanges {
                target: rangeSwitchInnerCircle;
                width: rangeSwitchOuterCircle.width * 0.5 - 1;
            }
            PropertyChanges {
                target: root;
                opacity: 1
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
                duration: 150
            }
            PropertyAnimation {
                target: rangeSwitchOuterCircle
                property: "opacity"
                duration: 150
            }
            PropertyAnimation {
                target: rangeSwitchInnerCircle
                property: "width"
                duration: 150
            }
        }
    ]

    Rectangle {
        id: rangeSwitchOuterCircle

        width: 10
        height: width
        x: (parent.width - width) * 0.5
        y: (parent.height - height) * 0.5
        radius: width * 0.5
        border.width: 1
        border.color: root.color
        color: "transparent"
        opacity: 0
    }

    Rectangle {
        id: rangeSwitchInnerCircle

        width: rangeSwitchOuterCircle.width
        height: width
        x: (parent.width - width) * 0.5
        y: (parent.height - height) * 0.5
        radius: width * 0.5
        color: root.color
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: root.softRangeActive = !root.softRangeActive
    }

    KisToolTip {
        id: toolTipPopup
        parent: root
        visible: root.enabled && mouseArea.containsMouse
    }
}
