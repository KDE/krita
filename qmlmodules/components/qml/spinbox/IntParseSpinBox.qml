/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/*
    \qmltype IntParseSpinBox
    A SpinBox with a ParseSpinboxContentItem. This will allow users to input
    simple maths expressions. This version is meant for integer input only.

    \qml
        IntParseSpinBox {
            from: 0
            to: 100
            value: 50

            prefix: i18nc("@label:spinbox", "Percentage: ")
            suffix: i18nc("@item:valuesuffix", "%")
        }
    \endqml
 */
SpinBox {
    id: root

    /*
        \qmlproperty prefix
        A string that will be prefixed to the current value.
    */
    property alias prefix: contentId.prefix
    /*
        \qmlproperty suffix
        A string that will be suffixed to the current value.
    */
    property alias suffix: contentId.suffix

    editable: true
    padding: 0
    focusPolicy: Qt.WheelFocus
    implicitWidth: contentId.implicitWidth + 1 + 2 + leftPadding + rightPadding

    contentItem: ParseSpinBoxContentItem {
        id: contentId

        anchors.fill: parent
        anchors.margins: 1
        anchors.rightMargin: 2
        focus: true

        value: root.value
        from: root.from
        to: root.to
        parentSpinBox: root

        onValueChanged: root.value = Math.round(value)
    }


    onValueChanged: contentId.value = root.value;

    Component.onCompleted: {

        // The following is to avoid an oddity inside Fusion
        // Where padding is added to ensure a centered text.
        if (root.mirrored) {
            root.rightPadding = 0;
        } else {
            root.leftPadding = 0;
        }
    }
}
