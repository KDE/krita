/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0

FocusScope {
    id: root

    property alias value: parseSpinBoxContentItem.value
    property alias decimals: parseSpinBoxContentItem.decimals
    property alias from: parseSpinBoxContentItem.from
    property alias to: parseSpinBoxContentItem.to
    property real softFrom: 0
    property real softTo: 0
    property alias showSoftRange: sliderOverlay.showTopBar
    property alias softRangeActive: sliderOverlay.topBarActive
    property real valueBeforeEditing: 0.0
    required property SpinBox parentSpinBox
    property alias prefix: parseSpinBoxContentItem.prefix
    property alias suffix: parseSpinBoxContentItem.suffix
    property alias focusPolicy: sliderManipulator.focusPolicy
    readonly property bool editing: parentSpinBox ? parentSpinBox.editable : false
    property alias blockUpdateSignalOnDrag: sliderManipulator.blockUpdateSignalOnDrag
    property alias exponentRatio: sliderManipulator.exponentRatio
    property alias fastSliderStep: sliderManipulator.fastSliderStep
    readonly property alias dragging : sliderManipulator.dragging

    implicitHeight: parseSpinBoxContentItem.implicitHeight
    implicitWidth: parseSpinBoxContentItem.implicitWidth + rangeSwitch.implicitWidth

    onSoftRangeActiveChanged: rangeSwitch.softRangeActive = root.softRangeActive

    KisSliderSpinBoxRangeSwitch {
        id: rangeSwitch

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        implicitWidth: root.showSoftRange ? 16 : 0
        visible: root.showSoftRange;
        color: root.parentSpinBox.palette.text
        //: @info:tooltip toggle between soft and hard range in the slider spin box
        toolTip: i18nc("@info:tooltip","Toggle between full range and subrange.\nFull range: [%1, %2]\nSubrange: [%3, %4]",
                       Number(root.from).toLocaleString(Qt.locale(), decimals),
                       Number(root.to).toLocaleString(Qt.locale(), decimals),
                       Number(root.softFrom).toLocaleString(Qt.locale(), decimals),
                       Number(root.softTo).toLocaleString(Qt.locale(), decimals));

        onSoftRangeActiveChanged: root.softRangeActive = softRangeActive
    }

    KisSliderOverlay {
        id: sliderOverlay
        
        property real value: parseSpinBoxContentItem.value

        anchors.fill: parseSpinBoxContentItem
        color: root.parentSpinBox.palette.highlight
        radius: 1
        topBarWidth: {
            const x = value - root.softFrom;
            const w = root.softTo - root.softFrom;
            return Math.max(Math.min(x, w), 0) / w;
        }
        bottomBarWidth: (value - root.from) / (root.to - root.from)
        faded: root.editing
        exponentRatio: root.exponentRatio
    }

    KisParseSpinBoxContentItem {
        id: parseSpinBoxContentItem
        
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: rangeSwitch.left
        parentSpinBox: root.parentSpinBox

        onEditingFinished: {
            if (root.parentSpinBox) {
                root.parentSpinBox.editable = false;
            }
            if (textInput.activeFocus) {
                sliderManipulator.forceActiveFocus(Qt.OtherFocusReason);
            }
        }

        onEditingCanceled: {
            if (root.parentSpinBox) {
                root.parentSpinBox.editable = false;
            }
            if (textInput.activeFocus) {
                sliderManipulator.forceActiveFocus(Qt.OtherFocusReason);
            }
            root.value = root.valueBeforeEditing;
        }

        onValueParsed: (v) => {
            if (!isNaN(v)) {
                sliderOverlay.value = v.toFixed(root.decimals);
            }
        }

        onValueChanged: {
            sliderOverlay.value = root.value;
            sliderManipulator.value = root.value;
        }

        Component.onCompleted: {
            textInput.horizontalAlignment = Text.AlignHCenter;
            textInput.focus = false;
            textInput.visible = Qt.binding(() => { return root.editing; });
        }
    }

    KisSliderSpinBoxManipulator {
        id: sliderManipulator
        anchors.fill: parseSpinBoxContentItem
        focus: true
        visible: !root.editing

        min: root.showSoftRange && root.softRangeActive ? root.softFrom : root.from
        max: root.showSoftRange && root.softRangeActive ? root.softTo : root.to
        stepSize: root.parentSpinBox ? root.parentSpinBox.stepSize : 1.0
        
        onValueChanged: root.value = value

        onEditingStarted: {
            if (root.parentSpinBox) {
                root.parentSpinBox.editable = true;
            }
            parseSpinBoxContentItem.textInput.selectAll();
            parseSpinBoxContentItem.textInput.forceActiveFocus(Qt.OtherFocusReason);
            root.valueBeforeEditing = root.value;
        }

        onEditingStartedWithValue: (v) => {
            if (root.parentSpinBox) {
                root.parentSpinBox.editable = true;
            }
            parseSpinBoxContentItem.textInput.selectAll();
            parseSpinBoxContentItem.textInput.forceActiveFocus(Qt.OtherFocusReason);
            root.valueBeforeEditing = root.value;
            parseSpinBoxContentItem.textInput.contentsText = v;
        }

        onIncrease: (wheel) => {
            if (!root.parentSpinBox) {
                return;
            }
            root.parentSpinBox.up.pressed = !wheel;
            root.parentSpinBox.increase();
        }

        onDecrease: (wheel) => {
            if (!root.parentSpinBox) {
                return;
            }
            root.parentSpinBox.up.pressed = !wheel;
            root.parentSpinBox.decrease();
        }

        Item {
            id: highlightedTextContainer
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: (root.showSoftRange && root.softRangeActive ? sliderOverlay.topBarWidth : sliderOverlay.bottomBarWidth) * sliderManipulator.width
            clip: true

            Text {
                id: highlightedText
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                width: sliderManipulator.width
                text: parseSpinBoxContentItem.textInput.text
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment: Text.AlignVCenter;
                font: root.parentSpinBox.font
                color: root.parentSpinBox.palette.highlightedText
            }
        }

        Item {
            id: nonHighlightedTextContainer
            anchors.top: parent.top
            anchors.left: highlightedTextContainer.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true

            Text {
                id: nonHighlightedText
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: sliderManipulator.width
                text: parseSpinBoxContentItem.textInput.text
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment: Text.AlignVCenter;
                color: root.parentSpinBox.palette.text
            }
        }
    }
}
