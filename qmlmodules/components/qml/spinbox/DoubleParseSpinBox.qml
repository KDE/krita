/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15

/*
    \qmltype DoubleParseSpinBox
    \inherits DoubleSpinBox
    A DoubleSpinBox with a ParseSpinboxContentItem. This will allow users to input
    simple maths expressions. This version is meant for double input only.

    \qml
        DoubleParseSpinBox {
            dFrom: 0
            dTo: 100.0
            dValue: 50.0
            decimals: 2

            prefix: i18nc("@label:spinbox", "Percentage: ")
            suffix: i18nc("@item:valuesuffix", "%")
        }
    \endqml


 */
DoubleSpinBox {
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

        value: root.dValue
        decimals: root.decimals
        from: root.dFrom
        to: root.dTo
        parentSpinBox: root

        onValueChanged: root.dValue = value
    }

    onDValueChanged: contentId.value = root.dValue
}
