/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import "sliderspinbox"

/*
    \qmltype DoubleSliderSpinBox
    A DoubleSpinBox with a ParseSpinBoxContentItem and a slider.
    This one only supports floating point values.

    Biggest difference between this and IntSliderSpinBox is that the value and
    range (from, to, softFrom and softTo) are prefixed with "d", and *must* be
    used instead of from/to/value. This is because those other values are
    reserved to be used by spinbox itself, and must be multiplied by the desired
    factor.

    NOTE: SliderSpinBoxes have no internal signal compressors, so it may be
    useful to implement those on the level of the data model to reduce updates,
    OR you can use \sa blockUpdateSignalOnDrag.

    \qml
        DoubleSliderSpinBox {
            id: spinbox
            dFrom: -100.0;
            dTo: 200.0;
            softDFrom: 0.0;
            softDTo: 100.0;
            value: 50.25;

            prefix: i18nc("@label:spinbox", "Percentage: ")
            suffix: i18nc("@item:valuesuffix", "%")
        }
    \endqml

   The value can be set by click and dragging with the mouse or pen or by
   typing in with the keyboard. To enter the edit mode, in which the keyboard
   can be used, one has to right-click inside the spinbox or click and hold
   the pointer inside or press the enter key. To leave the edit mode, one
   can press the enter key again, in which case the value is committed, or
   press the escape key, in which case the value is rejected.

   When dragging with the pointer, one can fine tune the value by dragging
   far away vertically from the spinbox. The farther the pointer is, the
   slower the value will change. If the pointer is inside the spinbox plus
   a certain margin, the value will not be scaled.
   By pressing the shift key the slow down will be even more pronounced and
   by pressing the control key the value will snap to the increment set by
   \sa fastSliderStep. The two keys can be used at the same time.

   A "soft range" can be set to make the slider display only a sub-range of the
   spinbox range. This way one can have a large range but display and set with
   the pointer and with more precision only the most commonly used sub-set
   of values.
   A value outside the "soft range" can be set by entering the edit
   mode and using the keyboard.
   The "soft range" is considered valid if the "soft maximum" is greater than
   the "soft minimum".
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

    /*
        \qmlproperty softDFrom
        Lower end of the soft range. Should be below softDTo and Dto,
        and below or equals to "DFrom".
    */
    property real softDFrom: dFrom
    /*
        \qmlproperty softDTo
        Upper end of the soft range. Should be above softDFrom and dFrom,
        and below or equals to dTo.
    */
    property real softDTo: dFrom

    /*
        \qmlproperty softRangeActive
        Whether the soft range is active.
    */
    property bool softRangeActive: false

    /*
        \qmlproperty blockUpdateSignalOnDrag
        Whether to block the value from being updates when dragging.
        When enabled, will only update the value on mouse release.
    */
    property alias blockUpdateSignalOnDrag: contentId.blockUpdateSignalOnDrag

    /*
        \qmlproperty exponentRatio
        Set the exponent used by a power function to modify the values
        as a function of the horizontal position.

        This allows having more values concentrated in one side of the
        slider than the other.
    */
    property alias exponentRatio: contentId.exponentRatio

    /*
        \qmlproperty fastSliderStep
        This sets the stepSize used when the user presses CTRL when modifying
        the slider.
    */
    property alias fastSliderStep: contentId.fastSliderStep

    /*
        \qmlproperty dragging

        Whether the slider is currently being dragged by the mouse.
    */
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
        
        SliderSpinBoxContentItem {
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
