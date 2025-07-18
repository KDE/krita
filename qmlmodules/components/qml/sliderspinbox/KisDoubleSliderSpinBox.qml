/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import org.krita.components 1.0

KisDoubleSpinBox {
    id: root

    property alias prefix: contentId.prefix
    property alias suffix: contentId.suffix
    property real softDFrom: dFrom
    property real softDTo: dFrom
    property bool softRangeActive: false
    property alias blockUpdateSignalOnDrag: contentId.blockUpdateSignalOnDrag
    property alias exponentRatio: contentId.exponentRatio
    property alias fastSliderStep: contentId.fastSliderStep
    readonly property alias dragging : contentId.dragging

    property bool isComplete: false;

    function fixSoftRange() {
        if (!isComplete) return;
        root.softDFrom = Math.min(Math.max(root.softDFrom, root.dFrom), root.dTo);
        root.softDTo = Math.min(Math.max(root.softDTo, root.softDFrom), root.dTo);
        contentId.showSoftRange = (root.softDFrom != root.softDTo)
                && (root.softDFrom !== root.dFrom || root.softDTo !== root.dTo);
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
            
            decimals: root.decimals
            from: root.dFrom
            to: root.dTo
            softFrom: root.softDFrom
            softTo: root.softDTo
            showSoftRange: false
            softRangeActive: root.softRangeActive
            focusPolicy: root.focusPolicy
            parentSpinBox: root

            onValueChanged: {
                if (!(blockUpdateSignalOnDrag && dragging) && root.dValue !== value) {
                    root.dValue = value;
                }
            }
            onDraggingChanged: {
                if (blockUpdateSignalOnDrag && !dragging) {
                    root.dValue = value;
                }
            }
        }
    }

    onDValueChanged: contentId.value = root.dValue;
    onDFromChanged: fixSoftRange()
    onDToChanged: fixSoftRange()
    onSoftDFromChanged: fixSoftRange()
    onSoftDToChanged: fixSoftRange()

    Component.onCompleted: {
        isComplete = true;
        fixSoftRange()
    }
}
