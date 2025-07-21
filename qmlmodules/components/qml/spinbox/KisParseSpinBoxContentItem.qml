/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import org.krita.components 1.0

FocusScope {
    id: root

    property real value: 0.0
    property int decimals: 0
    property real from: 0
    property real to: 0
    property alias prefix: textInput.prefix
    property alias suffix: textInput.suffix
    property alias warningOverlay: warningOverlay
    property alias textInput: textInput
    required property SpinBox parentSpinBox

    signal editingFinished()
    signal editingCanceled()
    signal valueParsed(real parsedValue)

    implicitHeight: textInput.implicitHeight
    implicitWidth: textMetrics.boundingRect.width + textInput.padding * 2
    
    onValueChanged: textInput.contentsText = Number(root.value).toLocaleString(Qt.locale(), 'f', decimals);

    onDecimalsChanged: textInput.contentsText = Number(root.value).toLocaleString(Qt.locale(), 'f', decimals);

    KisWarningOverlay {
        id: warningOverlay
        anchors.fill: parent
        clip: true

        radius: 1
        showWarningSign: width - textInput.displayTextWidth > 24 * (textInput.horizontalAlignment === Qt.AlignHCenter ? 2 : 1)
        warningSignAlignment: Qt.AlignRight
    }

    KisTextInputWithPrefixAndSuffix {
        id: textInput

        // This is a double, but QML 5.15 doesn't like it
        // when assigning a NaN to a double.
        property var parsedValue: 0.0

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

        KisNumParser { id: numParser;}

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
