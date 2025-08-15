/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import "../overlays"
import "../spinbox"

/*
    \qmltype SliderSpinBoxContentItem
    This is a FocusScope that enables a slider on a spinbox.

    \qml
        SpinBox {
            id: spinbox
            contentItem: SliderSpinBoxContentItem {
                id: parseItem
                parentSpinBox: spinbox
                anchors.fill: parent
                from: spinbox.from
                to: spinbox.to
                softFrom: from + 1
                softTo: to -1
                onValueChanged: spinbox.value = Math.round(value)

                prefix: i18nc("@label:spinbox", "Percentage: ")
                suffix: i18nc("@item:valuesuffix", "%")
            }

            onValueChanged: parseItem.value = value;
        }
    \endqml
 */
FocusScope {
    id: root

    /*
        \qmlproperty value
        The current value.
    */
    property alias value: parseSpinBoxContentItem.value
    /*
        \qmlproperty decimals
        Amount of decimals shown.
    */
    property alias decimals: parseSpinBoxContentItem.decimals
    /*
        \qmlproperty from
        Lower end of the hard range.
    */
    property alias from: parseSpinBoxContentItem.from
    /*
        \qmlproperty to
        Upper end of the hard range.
    */
    property alias to: parseSpinBoxContentItem.to
    /*
        \qmlproperty softFrom
        Lower end of the soft range. Should be below softTo and to,
        and below or equals to "from".
    */
    property real softFrom: 0
    /*
        \qmlproperty softTo
        Upper end of the soft range. Should be above softFrom and from,
        and below or equals to "to".
    */
    property real softTo: 0
    /*
        \qmlproperty showSoftRange
        Whether the softrange is shown. This is set whether the softrange
        and the hard range overlap.
    */
    property alias showSoftRange: sliderOverlay.showTopBar
    /*
        \qmlproperty softRangeActive
        Whether the soft range is active.
    */
    property alias softRangeActive: sliderOverlay.topBarActive
    /*
        \qmlproperty valueBeforeEditing
    */
    property real valueBeforeEditing: 0.0
    /*
        \qmlproperty parentSpinBox
        The SpinBox object that this is a content item of.
    */
    required property SpinBox parentSpinBox
    /*
        \qmlproperty prefix
        A string that will be prefixed to the current value.
    */
    property alias prefix: parseSpinBoxContentItem.prefix
    /*
        \qmlproperty suffix
        A string that will be suffixed to the current value.
    */
    property alias suffix: parseSpinBoxContentItem.suffix
    /*
        \qmlproperty focusPolicy
    */
    property alias focusPolicy: sliderManipulator.focusPolicy
    /*
        \qmlproperty editing
        returns true when the parentSpinBox is currently being edited.
    */
    readonly property bool editing: parentSpinBox ? parentSpinBox.editable : false
    /*
        \qmlproperty blockUpdateSignalOnDrag
        Whether to block the value from being updates when dragging.
    */
    property alias blockUpdateSignalOnDrag: sliderManipulator.blockUpdateSignalOnDrag

    /*
        \qmlproperty exponentRatio
        Set the exponent used by a power function to modify the values
        as a function of the horizontal position.

        This allows having more values concentrated in one side of the
        slider than the other.
    */
    property alias exponentRatio: sliderManipulator.exponentRatio

    /*
        \qmlproperty fastSliderStep
        This sets the stepSize used when the user presses CTRL when modifying
        the slider.
    */
    property alias fastSliderStep: sliderManipulator.fastSliderStep

    /*
        \qmlproperty dragging
        Whether the slider is currently being dragged by the mouse.
    */
    readonly property alias dragging : sliderManipulator.dragging

    implicitHeight: parseSpinBoxContentItem.implicitHeight
    implicitWidth: parseSpinBoxContentItem.implicitWidth + rangeSwitch.implicitWidth

    onSoftRangeActiveChanged: rangeSwitch.softRangeActive = root.softRangeActive

    SliderSpinBoxRangeSwitch {
        id: rangeSwitch

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        implicitWidth: root.showSoftRange ? 16 : 0
        visible: root.showSoftRange;
        color: root.parentSpinBox.palette.text
        //: @info:tooltip toggle between soft and hard range in the slider spin box
        toolTip: i18nc("@info:tooltip","Toggle between full range and subrange.\nFull range: [%1, %2]\nSubrange: [%3, %4]",
                       Number(root.from).toLocaleString(Qt.locale(), 'f', decimals),
                       Number(root.to).toLocaleString(Qt.locale(), 'f', decimals),
                       Number(root.softFrom).toLocaleString(Qt.locale(), 'f', decimals),
                       Number(root.softTo).toLocaleString(Qt.locale(), 'f', decimals));

        onSoftRangeActiveChanged: root.softRangeActive = softRangeActive
    }

    SliderOverlay {
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

    ParseSpinBoxContentItem {
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

    SliderSpinBoxManipulator {
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
