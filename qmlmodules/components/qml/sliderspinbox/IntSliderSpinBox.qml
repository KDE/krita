/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

/*
    \qmltype IntSliderSpinBox
    A SpinBox with a ParseSpinBoxContentItem and a slider.
    This one only supports integers.

    NOTE: SliderSpinBoxes have no internal signal compressors, so it may be
    useful to implement those on the level of the data model to reduce updates,
    OR you can use \sa blockUpdateSignalOnDrag.

    \qml
        IntSliderSpinBox {
            id: spinbox
            from: 0;
            to: 100;
            value: 50;

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

    /*
        \qmlproperty softFrom
        Lower end of the soft range. Should be below softTo and to,
        and below or equals to "from".
    */
    property int softFrom: from

    /*
        \qmlproperty softTo
        Upper end of the soft range. Should be above softFrom and from,
        and below or equals to "to".
    */
    property int softTo: from

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
    property int fastSliderStep: 5

    /*
        \qmlproperty dragging
        Whether the slider is currently being dragged by the mouse.
    */
    readonly property alias dragging : contentId.dragging

    property bool isComplete: false;

    function fixSoftRange() {
        if (!isComplete) return;
        root.softFrom = Math.min(Math.max(root.softFrom, root.from), root.To);
        root.softTo = Math.min(Math.max(root.softTo, root.softFrom), root.To);
        contentId.showSoftRange = (root.softFrom !== root.softTo)
                && (root.softFrom !== root.from || root.softTo !== root.to);
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

            onValueChanged: {
                if (!(blockUpdateSignalOnDrag && dragging) && root.value !== value) {
                    root.value = value;
                }
            }
            onDraggingChanged: {
                if (blockUpdateSignalOnDrag && !dragging) {
                    root.value = value;
                }
            }
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
        isComplete = true;
        fixSoftRange()
    }
}
