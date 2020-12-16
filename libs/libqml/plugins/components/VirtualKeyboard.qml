/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Rectangle {
    id: base;

    property bool keyboardVisible: false;
    onKeyboardVisibleChanged: if (!keyboardVisible) keys.mode = KeyboardModel.NormalMode;

    anchors.left: parent.left;
    anchors.right: parent.right;

    y: parent.height;
    height: parent.height * 0.45;
    color: "black";

    MouseArea {
        anchors.fill: parent;
        onClicked: { }
    }
    SimpleTouchArea {
        anchors.fill: parent;
        onTouched: { }
    }

   Flow {
       visible: keys.useBuiltIn;
       anchors.fill: parent;
       anchors.topMargin: 4;
       anchors.leftMargin: Constants.GridWidth * 1.5;
       anchors.rightMargin: Constants.GridWidth * 1.5;
       anchors.bottomMargin: 4;

       Repeater {
           model: keys;
           delegate: keyDelegate;
       }
   }

    states: State {
        name: "visible";
        PropertyChanges { target: base; y: base.parent.height * 0.55; keyboardVisible: true; }
    }

    transitions: Transition {
        reversible: true;
        SequentialAnimation {
            NumberAnimation { properties: "y"; }
            PropertyAction { property: "keyboardVisible"; }
        }
    }

    Connections {
        target: Krita.VirtualKeyboardController;

        onShowKeyboard: {
            base.state = "visible";
        }
        onHideKeyboard: base.state = "";
    }

    Connections {
        target: Settings;

        onFocusItemChanged: {
            if (Settings.focusItem != null && Settings.focusItem != undefined) {
                if (Settings.focusItem.text == "") {
                    keys.mode = KeyboardModel.CapitalMode;
                }
                if (Settings.focusItem.numeric != undefined && Settings.focusItem.numeric === true) {
                    keys.mode = KeyboardModel.NumericMode;
                }
            }
        }
    }

    KeyboardModel {
        id: keys;
    }

    Component {
        id: keyDelegate;

        Item {
            width: (Constants.GridWidth * 0.75) * model.width;
            height: (base.height - 8) / 4;

            Button {
                anchors {
                    left: parent.left;
                    right: parent.right;
                    top: parent.top;
                    margins: 4;
                }

                height: model.keyType == KeyboardModel.EnterKey && keys.mode == KeyboardModel.NumericMode ? parent.height * 2 - 4: parent.height - 4;

                border.width: model.keyType == KeyboardModel.SpacerKey ? 0 : 2;
                border.color: "white";
                radius: 8;

                color: {
                    if (model.keyType == KeyboardModel.ShiftKey && keys.mode == KeyboardModel.CapitalMode) {
                        return "#666666";
                    } else if (model.keyType == KeyboardModel.NumericModeKey && keys.mode == KeyboardModel.NumericMode) {
                        return "#666666";
                    } else {
                        return "transparent";
                    }
                }

                text: model.text;
                textColor: model.keyType != KeyboardModel.SpacerKey ? "white" : "#333333";

                highlight: model.keyType != KeyboardModel.SpacerKey ? true : false;
                highlightColor: "#666666";

                onClicked: {
                    switch(model.keyType) {
                        case KeyboardModel.BackspaceKey:
                            Settings.focusItem.text = Settings.focusItem.text.substring(0, Settings.focusItem.text.length - 1);
                            break;
                        case KeyboardModel.EnterKey:
                            base.state = "";
                            break;
                        case KeyboardModel.ShiftKey:
                            keys.mode = keys.mode != KeyboardModel.CapitalMode ? KeyboardModel.CapitalMode : KeyboardModel.NormalMode;
                            break;
                        case KeyboardModel.LeftArrowKey:
                            Settings.focusItem.cursorPosition -= 1;
                            break;
                        case KeyboardModel.RightArrowKey:
                            Settings.focusItem.cursorPosition += 1;
                            break;
                        case KeyboardModel.NumericModeKey:
                            keys.mode = keys.mode != KeyboardModel.NumericMode ? KeyboardModel.NumericMode : KeyboardModel.NormalMode;
                            break;
                        case KeyboardModel.NormalKey:
                            Settings.focusItem.text += model.text;
                            if (keys.mode == KeyboardModel.CapitalMode) {
                                keys.mode = KeyboardModel.NormalMode;
                            }
                            break;
                        case KeyboardModel.CloseKey:
                            base.state = "";
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
}
