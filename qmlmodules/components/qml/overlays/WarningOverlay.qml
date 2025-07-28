/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15

/*
    \qmltype WarningOverlay
    This item is an overlay for the parse spinboxes, when enabled
    it shows a bright orange-red overlay with a warning sign.

    It handles animating the changes for showing this warning as well.
 */
Item {
    id: root

    /*
        \qmlproperty warn
        Whether to start warning. When enabled the overlay will show itself.
    */
    property bool warn: false
    /*
        \qmlproperty warningSignAlignment
        Whether the warning sign is at the left or right of the overlay.
        By default this is Qt.AlignLeft.
    */
    property int warningSignAlignment: Qt.AlignLeft
    /*
        \qmlproperty showWarningSign
        Whether to show the warning sign.
    */
    property bool showWarningSign: true
    /*
        \qmlproperty radius
        the radius of the overlay rectangle.
    */
    property alias radius: background.radius

    states: [
        State {
            name: "warning"
            when: root.warn && !root.showWarningSign

            PropertyChanges {
                target: background;
                opacity: 0.6
            }
            PropertyChanges {
                target:warningSign
                opacity: 0
                x: root.warningSignAlignment == Qt.AlignLeft ? -24 : root.width
            }
        },
        State {
            name: "warningWithSign"
            when: root.warn && root.showWarningSign

            PropertyChanges {
                target: background;
                opacity: 0.6
            }

            PropertyChanges {
                target:warningSign
                opacity: 1
                x: root.warningSignAlignment == Qt.AlignLeft ? 0 : root.width - 24
            }
        }
    ]

    transitions: [
        Transition {
            from: ""
            to: "warning"
            PropertyAnimation {
                target: background
                property: "opacity"
                duration: 250
            }
        },
        Transition {
            from: ""
            to: "warningWithSign"
            ParallelAnimation {
                PropertyAnimation {
                    target: background
                    property: "opacity"
                    duration: 250
                }
                PropertyAnimation {
                    target: warningSign
                    properties: "opacity, x"
                    duration: 250
                }
            }
        },
        Transition {
            from: "warning"
            to: ""
            PropertyAnimation {
                target: background
                property: "opacity"
                duration: 250
            }
        },
        Transition {
            from: "warning"
            to: "warningWithSign"
            PropertyAnimation {
                target: warningSign
                properties: "opacity, x"
                duration: 250
            }
        },
        Transition {
            from: "warningWithSign"
            to: ""
            PropertyAnimation {
                target: background
                property: "opacity"
                duration: 250
            }
            PropertyAnimation {
                target: warningSign
                properties: "opacity, x"
                duration: 250
            }
        },
        Transition {
            from: "warningWithSign"
            to: "warning"
            PropertyAnimation {
                target: warningSign
                properties: "opacity, x"
                duration: 250
            }
        }
    ]

    Rectangle {
        id: background
        anchors.fill: parent
        color: Qt.rgba(1, 0.2, 0, 1)
        opacity: 0
    }

    Item {
        id: warningSign
        x: root.warningSignAlignment == Qt.AlignLeft ? -24 : root.width
        anchors.verticalCenter: parent.verticalCenter
        width: 24
        height: 24
        opacity: 0
        
        Image {
            anchors.fill: parent
            anchors.margins: 4
            source: "qrc:///16_light_warning.svg"
        }
    }
}
