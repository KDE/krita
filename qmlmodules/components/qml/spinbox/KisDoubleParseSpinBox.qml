/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15

KisDoubleSpinBox {
    id: root

    property alias prefix: contentId.prefix
    property alias suffix: contentId.suffix
    
    editable: true
    padding: 0
    focusPolicy: Qt.WheelFocus

    implicitWidth: contentId.implicitWidth + 1 + 2 + leftPadding + rightPadding

    contentItem: FocusScope {
        KisParseSpinBoxContentItem {
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
    }

    onDValueChanged: contentId.value = root.dValue
}
