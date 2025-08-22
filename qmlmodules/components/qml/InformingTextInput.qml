/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0 as Kis
import "overlays"

/**
  \qmltype InformingTextInput

  This is a FocusScope containing a text input and a warningOverlay.

    \qml
        Button {
            id: button;
            contentItem: InformingTextInput {
                id: textInput;
                onEditingFinished: {
                    let valid: button.validate(textInput.text);
                    if (valid) {
                        stopWarning()
                    } else {
                        startWarning()
                    }
                }
            }

            /// Assume this does something more useful than check
            /// if text is literally the string "Valid".
            funcion validate(text) {
                text == "Valid";
            }
        }
    \endqml

  */
TextField {
    id: root

    // Forwarded aliases.

    /*
        \qmlproperty backgroundColor
        Color of the background.
    */
    property alias backgroundColor: bgColor.color;

    /*
        \qmlproperty backgroundBorderColor
        Color of the border when not in focus.
    */
    property color backgroundBorderColor: theme.window.midShadeColor;

    /*
        \qmlproperty warn
        Whether the warning overlay is visible;
    */
    property alias warn: warningOverlay.warn;
    /*
        \qmlproperty warnColor
        Color of the warn bgcolor, defaults to negativeTextColor;
    */
    property alias warnColor: warningOverlay.color;
    /*
        \qmlproperty warnColorBorder
        Color of the border when warning, defaults to negativeTextColor;
    */
    property color warnColorBorder: theme.window.negativeTextColor;

    /*
        \qmlproperty warningTimeOut
        The time out till it shows the warning. This is 2000 ms by default
        to ensure that the warning doesn't pop-up too early.
    */
    property alias warningTimeOut: warningTimer.interval;

    color: theme.view.textColor;
    selectedTextColor: theme.selection.textColor;
    selectionColor: theme.selection.backgroundColor;
    /// Start warning. This calls a timer.
    function startWarning() {
        warningTimer.start();
    }
    /// Stop warning.
    function stopWarning() {
        warningTimer.stop();
        warn = false;
    }

    Kis.Theme {
        id: theme;
        window.state: root.enabled? SystemPalette.Active: SystemPalette.Disabled;
        button.state: root.enabled? SystemPalette.Active: SystemPalette.Disabled;
        selection.state: root.enabled? SystemPalette.Active: SystemPalette.Disabled;
    }

    background: Rectangle {
        id: bgColor;
        radius: 1;
        color: theme.view.backgroundColor;
        WarningOverlay {
            id: warningOverlay;
            anchors.fill: parent;
            clip: true;

            radius: bgColor.radius;
            showWarningSign: width - textMetrics.boundingRect.width > 24 * (root.horizontalAlignment === Qt.AlignHCenter ? 2 : 1)
            warningSignAlignment: Qt.AlignRight;
            color: theme.window.negativeBackgroundColor;
        }

        Rectangle {
            anchors.fill: parent;
            color: "transparent";
            radius: bgColor.radius;
            border.width: 1;
            border.color: warningOverlay.warn? root.warnColorBorder: root.activeFocus ? theme.view.focusColor : backgroundBorderColor;
        }
    }

    Timer {
        id: warningTimer
        interval: 2000;
        onTriggered: warningOverlay.warn = true
    }

    TextMetrics {
        id: textMetrics
        font: root.font
        text: root.text;
    }
}
