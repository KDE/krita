/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0
import "KisAngleSelectorUtil.js" as KisAngleSelectorUtil

FocusScope {
    id: root

    enum FlipOptionsMode
    {
        NoFlipOptions,
        MenuButton,
        Buttons,
        ContextMenu
    }

    property alias angle: spinBox.dValue
    property alias decimals: spinBox.decimals
    property alias from: spinBox.dFrom
    property alias to: spinBox.dTo
    property alias prefix: spinBox.prefix
    property alias suffix: spinBox.suffix
    property alias wrap: spinBox.wrap
    property alias snapAngle: angleGauge.snapAngle
    property alias defaultAngle: angleGauge.defaultAngle
    property alias increaseClockwise: angleGauge.increaseClockwise

    property int flipOptionsMode: KisAngleSelector.FlipOptionsMode.Buttons
    property int gaugeSize: 0
    property bool flatSpinBox: false

    function reset()
    {
        angleGauge.reset();
    }

    function closestCoterminalAngleInRange(angle: real) : real
    {
        return KisAngleSelectorUtil.closestCoterminalAngleInRange(angle, root.from, root.to);
    }

    function flip(horizontal: bool, vertical: bool)
    {
        let ok = true;
        let flippedAngle = KisAngleSelectorUtil.flipAngleInRange(root.angle, root.from, root.to, horizontal, vertical,
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

        KisAngleGauge {
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

        KisDoubleParseSpinBox {
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
