/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

SpinBox {
    id: root

    property alias prefix: contentId.prefix
    property alias suffix: contentId.suffix
    property int softFrom: from
    property int softTo: from
    property bool softRangeActive: false
    property alias blockUpdateSignalOnDrag: contentId.blockUpdateSignalOnDrag
    property alias exponentRatio: contentId.exponentRatio
    property int fastSliderStep: 5
    readonly property alias dragging : contentId.dragging

    function fixSoftRange() {
        if (root.softFrom < root.from) {
            root.softFrom = root.from;
        } else if (root.softFrom > root.softTo) {
            root.softFrom = root.softTo;
        }
        if (root.softTo < root.softFrom) {
            root.softTo = root.softFrom;
        } else if (root.softTo > root.to) {
            root.softTo = root.to;
        }
        contentId.showSoftRange = root.softFrom !== root.softTo;
    }

    editable: false
    padding: 0
    focusPolicy: Qt.WheelFocus
    implicitWidth: contentId.implicitWidth + 1 + 2 + leftPadding + rightPadding

    contentItem: FocusScope {
        focus: true

        KisSliderSpinBoxContentItem {
            id: contentId
            
            anchors.fill: parent
            anchors.margins: 1
            anchors.rightMargin: 2
            focus: true

            from: root.from
            to: root.to
            softFrom: root.softFrom
            softTo: root.softTo
            showSoftRange: false
            softRangeActive: root.softRangeActive
            parentSpinBox: root
            focusPolicy: root.focusPolicy
            fastSliderStep: root.fastSliderStep

            onValueChanged: root.value = Math.round(value)
        }
    }

    onValueChanged: contentId.value = root.value
    onFromChanged: fixSoftRange()
    onToChanged: fixSoftRange()
    onSoftFromChanged: fixSoftRange()
    onSoftToChanged: fixSoftRange()

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
