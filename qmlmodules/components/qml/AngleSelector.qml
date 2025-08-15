/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import "angleselector"
import "tooltip"
import "angleselector/AngleSelectorUtil.js" as AngleSelectorUtil

/*
    \qmltype AngleSelector
    A FocusScope that combines a AngleGauge and a DoubleSpinBox into one
    convenient widget.

    \qml
        AngleSelector {
            angle: 45;
            deimals: 2;
            from: -180;
            to: 180;
            snapAngle: 5;
            defaultAngle 0;
            increaseClockwise: true;

            prefix: i18nc("@label:spinbox", "Angle: ")
            suffix: i18nc("@item:valuesuffix", "°")

            flipOptionsMode: AngleSelector.Buttons
            flatSpinBox: false;
        }
    \endqml
 */
FocusScope {
    id: root

    enum FlipOptionsMode
    {
        NoFlipOptions, ///< There is no flip options available
        MenuButton,  ///< The flip options are shown as a menu accessible via a options button
        Buttons, ///< The flip options are shown as individual buttons
        ContextMenu ///< The flip options are shown only as a context menu when
                    ///< right-clicking the gauge widget.
                    ///< The options are shown in the context menu also if the mode is
                    ///< FlipOptionsMode_MenuButton or FlipOptionsMode_Buttons but with this
                    ///< mode there will be no additional buttons
    }

    /*
        \qmlproperty angle
        Current angle value.
    */
    property alias angle: spinBox.dValue

    /*
        \qmlproperty decimals
        Amount of decimals to show.
        /sa DoubleSpinBox
    */
    property alias decimals: spinBox.decimals

    /*
      \qmlproperty from
      Lower end of the angle range. Is a double.
      */
    property alias from: spinBox.dFrom

    /*
      \qmlproperty to
      Upper end of the angle range. Is a double.
      */
    property alias to: spinBox.dTo

    /*
        \qmlproperty prefix
        A string that will be prefixed to the current value.
    */
    property alias prefix: spinBox.prefix

    /*
        \qmlproperty suffix
        A string that will be suffixed to the current value.
    */
    property alias suffix: spinBox.suffix

    /*
      \qmlproperty wrap
      Whether the spinbox is wrapping.
      */
    property alias wrap: spinBox.wrap

    /*
        \qmlproperty snapAngle
        Gets the angle to which multiples the selected angle will snap.

        The default snap angle is 15 degrees so the selected angle will snap
        to its multiples (0, 15, 30, 45, etc.)
    */
    property alias snapAngle: angleGauge.snapAngle

    /*
        \qmlproperty defaultAngle
        Gets the angle that is used to reset the current angle.
        This angle is used when the user double clicks on the widget.
    */
    property alias defaultAngle: angleGauge.defaultAngle

    /*
        \qmlproperty increaseClockwise
        Whether the angle gauge increase clockwise.
        Otherwise, increases counter clockwise.
    */
    property alias increaseClockwise: angleGauge.increaseClockwise

    /*
      \qmlproperty flipOptionsMode
      How the flipOptions are presented.

      \sa FlipOptionsMode enum.
      */
    property int flipOptionsMode: AngleSelector.FlipOptionsMode.Buttons

    /*
      \qmlproperty gaugeSize

      Set the explicit size for the gauge. Will otherwise use the implicit size.
      */
    property int gaugeSize: 0

    /*
      \qmlproperty flatSpinBox

      Gets if the spin box is flat (no border or background).
      */
    property bool flatSpinBox: false

    function reset()
    {
        angleGauge.reset();
    }

    function closestCoterminalAngleInRange(angle: real) : real
    {
        return AngleSelectorUtil.closestCoterminalAngleInRange(angle, root.from, root.to);
    }

    function flip(horizontal: bool, vertical: bool)
    {
        let ok = true;
        let flippedAngle = AngleSelectorUtil.flipAngleInRange(root.angle, root.from, root.to, horizontal, vertical,
                                                                 () => { ok = false; });
        if (ok) {
            root.angle = flippedAngle;
        }
    }

    implicitWidth: row.implicitWidth
    implicitHeight: row.implicitHeight

    Action {
        id: actionFlipHorizontally
        //: Flips the angle horizontally, around the vertical axis
        text: qsTr("Flip the angle horizontally")
        icon.source: "qrc:///light_flip_angle_h.svg"
        icon.width: 20
        icon.height: 20
        onTriggered: root.flip(true, false)
    }

    Action {
        id: actionFlipVertically
        //: Flips the angle vertically, around the horizontal axis
        text: qsTr("Flip the angle vertically")
        icon.source: "qrc:///light_flip_angle_v.svg"
        icon.width: 20
        icon.height: 20
        onTriggered: root.flip(false, true)
    }

    Action {
        id: actionFlipHorizontallyAndVertically
        //: Flips the angle horizontally and vertically
        text: qsTr("Flip the angle horizontally and vertically")
        icon.source: "qrc:///light_flip_angle_hv.svg"
        icon.width: 20
        icon.height: 20
        onTriggered: root.flip(true, true)
    }

    Action {
        id: actionResetAngle
        //: Resets the angle
        text: qsTr("Reset Angle")
        onTriggered: root.reset()
    }

    Menu {
        id: menu
        property Item separator: MenuSeparator {}
        contentData: [
            actionFlipHorizontally,
            actionFlipVertically,
            actionFlipHorizontallyAndVertically,
            separator,
            actionResetAngle
        ]
    }

    Row {
        id: row

        spacing: 5
        anchors.fill: parent

        AngleGauge {
            id: angleGauge

            anchors.verticalCenter: parent.verticalCenter
            implicitWidth: root.gaugeSize > 0 ? root.gaugeSize : spinBox.implicitHeight
            implicitHeight: implicitWidth

            onAngleChanged: spinBox.dValue = root.closestCoterminalAngleInRange(angle)

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    if (root.flipOptionsMode === KisAngleSelector.FlipOptionsMode.ContextMenu) {
                        menu.popup();
                    }
                }
            }
        }

        DoubleParseSpinBox {
            id: spinBox

            anchors.verticalCenter: parent.verticalCenter
            wrap: true
            dFrom: 0
            dTo: 360
            hoverEnabled: true
            suffix: "º"


            onValueChanged: angleGauge.angle = dValue

            Component.onCompleted: {
                spinBox.background.visible = Qt.binding(() => { return !root.flatSpinBox || spinBox.hovered; });
                spinBox.up.indicator.visible = Qt.binding(() => { return spinBox.background.visible });
                spinBox.down.indicator.visible = Qt.binding(() => { return spinBox.background.visible });
            }
        }

        Row {
            id: buttonsContainer

            anchors.verticalCenter: parent.verticalCenter
            spacing: 0

            ToolButton {
                id: buttonMenu
                visible: root.flipOptionsMode === KisAngleSelector.FlipOptionsMode.MenuButton
                padding: 4
                display: AbstractButton.IconOnly
                icon.source: "qrc:///light_view-choose.svg"
                icon.width: 16
                icon.height: 16
                icon.color: palette.buttonText;
                onClicked: {
                    menu.popup(buttonMenu, 0, buttonMenu.height);
                    menu.forceActiveFocus();
                }
            }

            ToolButton {
                id: buttonFlipHorizontally
                visible: root.flipOptionsMode === KisAngleSelector.FlipOptionsMode.Buttons
                padding: 4
                display: AbstractButton.IconOnly
                action: actionFlipHorizontally
                icon.color: palette.buttonText;
                KisToolTip {
                    parentControl: buttonFlipHorizontally
                    text: actionFlipHorizontally.text
                }
            }

            ToolButton {
                id: buttonFlipVertically
                visible: root.flipOptionsMode === KisAngleSelector.FlipOptionsMode.Buttons
                padding: 4
                display: AbstractButton.IconOnly
                action: actionFlipVertically
                icon.color: palette.buttonText;
                KisToolTip {
                    parentControl: buttonFlipVertically
                    text: actionFlipVertically.text
                }
            }

            ToolButton {
                id: buttonFlipHorizontallyAndVertically
                visible: root.flipOptionsMode === KisAngleSelector.FlipOptionsMode.Buttons
                padding: 4
                display: AbstractButton.IconOnly
                action: actionFlipHorizontallyAndVertically
                icon.color: palette.buttonText;
                KisToolTip {
                    parentControl: buttonFlipHorizontallyAndVertically
                    text: actionFlipHorizontallyAndVertically.text
                }
            }
        }
    }
}
