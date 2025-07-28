/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0 as Kis

/*
    \qmltype ParseSpinBoxContentItem
    A FocusScope that allows for parsing math expressions via KisNumParser,
    in addtion to allowing for suffix and prefix.

    When the math expression is incorrect, a WarningOverlay will be drawn.

    ParseSpinBoxContentItem is meant to be used as the contentItem of a Spinbox.

    \qml
        SpinBox {
            id: spinbox
            contentItem: ParseSpinBoxContentItem {
                id: parseItem
                parentSpinBox: spinbox
                anchors.fill: parent
                from: spinbox.from
                to: spinbox.to
                onValueChanged: spinbox.value = Math.round(value)
            }

            onValueChanged: parseItem.value = value;
        }
    \endqml
 */
FocusScope {
    id: root

    /*
        \qmlproperty value
        current value as qreal.
    */
    property real value: 0.0
    /*
        \qmlproperty decimals
        Number of visible decimals.
    */
    property int decimals: 0
    /*
        \qmlproperty from
        Real representing the lower end of the value range.
    */
    property real from: 0
    /*
        \qmlproperty to
        Real representing the upper end of the value range.
    */
    property real to: 0
    /*
        \qmlproperty prefix
        A string that will be prefixed to the current value.
    */
    property alias prefix: textInput.prefix
    /*
        \qmlproperty suffix
        A string that will be suffixed to the current value.
    */
    property alias suffix: textInput.suffix
    /*
        \qmlproperty warningOverlay
        The WarningOverlay object.
    */
    property alias warningOverlay: warningOverlay
    /*
        \qmlproperty textInput
        The TextInputWithPrefixAndSuffix object.
    */
    property alias textInput: textInput
    /*
        \qmlproperty parentSpinBox
        The SpinBox object that this is a content item of.
    */
    required property SpinBox parentSpinBox

    /*
        This signal is emitted when editing has finished on the text input.
    */
    signal editingFinished()

    /*
        This signal is emitted when editing has been canceled on the text input.
    */
    signal editingCanceled()

    /*
        This signal is emitted when the text input value has been parsed successfully.
    */
    signal valueParsed(real parsedValue)

    implicitHeight: textInput.implicitHeight
    implicitWidth: textMetrics.boundingRect.width + textInput.padding * 2
    
    onValueChanged: textInput.contentsText = Number(root.value).toLocaleString(Qt.locale(), 'f', decimals);

    onDecimalsChanged: textInput.contentsText = Number(root.value).toLocaleString(Qt.locale(), 'f', decimals);

    Kis.WarningOverlay {
        id: warningOverlay
        anchors.fill: parent
        clip: true

        radius: 1
        showWarningSign: width - textInput.displayTextWidth > 24 * (textInput.horizontalAlignment === Qt.AlignHCenter ? 2 : 1)
        warningSignAlignment: Qt.AlignRight
    }

    Kis.TextInputWithPrefixAndSuffix {
        id: textInput
        property double parsedValue: 0.0

        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        padding: 4
        contentsText: Number(root.value).toLocaleString(Qt.locale(), 'f', decimals);
        focus: true
        font: root.parentSpinBox.font
        color: root.parentSpinBox.palette.text
        selectionColor: root.parentSpinBox.palette.highlight
        selectedTextColor: root.parentSpinBox.palette.highlightedText
        inputMethodHints: root.parentSpinBox.inputMethodHints
        readOnly: !root.parentSpinBox.editable

        onEditingFinished: {
            warningTimer.stop();
            if (isNaN(parsedValue)) {
                warningOverlay.warn = true;
            } else {
                warningOverlay.warn = false;
                root.value = parsedValue;
                root.editingFinished();
            }
        }

        Kis.KisNumParser { id: numParser;}

        onContentsTextChanged: {
            parsedValue = numParser.parseSimpleMathExpr(contentsText);
            warningOverlay.warn = false;
            if (isNaN(parsedValue)) {
                warningTimer.restart();
            } else {
                warningTimer.stop();
            }
            root.valueParsed(parsedValue)
        }

        Keys.onEscapePressed: {
            warningTimer.stop();
            warningOverlay.warn = false;
            textInput.contentsText = Number(root.value).toLocaleString(Qt.locale(), 'f', decimals);
            root.editingCanceled();
        }

        Timer {
            id: warningTimer
            interval: 2000;
            onTriggered: warningOverlay.warn = true
        }

        MouseArea {
            property real cumulatedTrackpadLength: 0.0

            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: textInput.readOnly ? Qt.ArrowCursor : Qt.IBeamCursor

            onWheel: (we) => {
                if (root.parentSpinBox.focusPolicy & Qt.WheelFocus) {
                    if (!textInput.activeFocus) {
                        textInput.forceActiveFocus(Qt.MouseFocusReason);
                        textInput.selectAll();
                    }
                }

                let inc = 0.0
                if (we.pixelDelta && we.pixelDelta.y !== 0) {
                    cumulatedTrackpadLength += we.pixelDelta.y;
                    if (Math.abs(cumulatedTrackpadLength) > 30) {
                        inc = cumulatedTrackpadLength;
                        cumulatedTrackpadLength = 0.0;
                    }
                } else {
                    inc = we.angleDelta.y;
                }

                inc *= we.inverted ? -1 : 1; 
                if (inc > 0) {
                    root.parentSpinBox.increase();
                    textInput.selectAll();
                } else if (inc < 0) {
                    root.parentSpinBox.decrease();
                    textInput.selectAll();
                }
            }
        }
    }

    TextMetrics {
        id: textMetrics
        font: textInput.font
        text: {
            const fromText = Number(root.from).toLocaleString(Qt.locale(), 'f', decimals);
            const toText = Number(root.to).toLocaleString(Qt.locale(), 'f', decimals);
            return root.prefix
                    + (fromText.length > toText.length ? fromText : toText)
                    + root.suffix
        }
    }
}
