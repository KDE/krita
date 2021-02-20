/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

Item {
    id: base;

    property bool enabled: true;
    property alias placeholder: textField.placeholder;
    property real value: 0;
    property real min: 0;
    property real max: 1000;
    property int decimals: 2;
    property alias useExponentialValue: valueSlider.useExponentialValue;

    height: textField.height + valueSlider.height;

    property alias border: valueSlider.border;
    property alias background: valueSlider.background;
    property alias model: predefinedList.model;
    property bool expanded: false;
    onExpandedChanged: {
        if(expanded === true) {
            base.state = "expanded";
        }
        else {
            base.state = "";
        }
    }
    states: [
        State {
            name: "expanded";
            PropertyChanges { target: predefinedListFlow; height: predefinedListFlow.childrenRect.height; }
            PropertyChanges { target: base; height: textField.height + predefinedListFlow.height + valueSlider.height; }
        }
    ]
    transitions: [
        Transition {
            to: "*"
            ParallelAnimation {
                PropertyAnimation { target: predefinedListFlow; property: "height"; duration: Constants.AnimationDuration; }
                PropertyAnimation { target: base; property: "height"; duration: Constants.AnimationDuration; }
            }
        }
    ]

    onMinChanged: d.fixHandle();
    onMaxChanged: d.fixHandle();
    onValueChanged: {
        if (decimals === 0) {
            if (value !== Math.round(value))
            {
                value = Math.round(value);
                return;
            }
        }
        else if (value * Math.pow(10, decimals) !== Math.round(value * Math.pow(10, decimals))) {
            value = Math.round(value * Math.pow(10, decimals)) / Math.pow(10, decimals);
            return;
        }
        if (value < min) {
            value = min;
            return;
        }
        if (value > max) {
            value = max;
            return;
        }
        if (textField.text != value) {
            textField.text = value.toFixed(decimals);
        }
        if (useExponentialValue) {
             if (valueSlider.exponentialValue !== value) {
                 valueSlider.exponentialValue = ( (value - min) / (max - min) ) * 100;
             }
        }
        else {
            if (valueSlider.value !== value) {
                valueSlider.value = ( (value - min) / (max - min) ) * 100;
            }
        }
    }

    Image {
        id: arrowsList
        anchors {
            right: parent.right;
            rightMargin: Constants.DefaultMargin;
            top: parent.top;
            topMargin: Constants.DefaultMargin;
        }
        height: textField.height - Constants.DefaultMargin * 2;
        width: height;
        source: Settings.theme.icon("expansionmarker");
        smooth: true
        rotation: base.expanded ? 0 : 90;
        Behavior on rotation { PropertyAnimation { duration: Constants.AnimationDuration; } }
        MouseArea {
            anchors.fill: parent;
            onClicked: base.expanded = !base.expanded;
        }
    }
    PanelTextField {
        id: textField
        anchors {
            top: parent.top;
            left: parent.left;
            right: arrowsList.left;
        }
        onFocusLost: value = text;
        onAccepted: value = text;
        numeric: true;

        border.width: valueSlider.border.width;
        border.color: valueSlider.border.color;
        background: valueSlider.background;
    }
    Flow {
        id: predefinedListFlow;
        clip: true;
        anchors {
            top: textField.bottom;
            left: parent.left;
            right: parent.right;
            leftMargin: Constants.DefaultMargin;
            rightMargin: Constants.DefaultMargin;
        }
        height: 0;
        Repeater {
            id: predefinedList;
            delegate: Button {
                width: predefinedListFlow.width / 3;
                height: width;
                text: model.value;
                onClicked: base.value = model.value;
            }
        }
    }
    Slider {
        id: valueSlider;
        anchors {
            top: predefinedListFlow.bottom;
            left: parent.left;
            right: parent.right;
            leftMargin: Constants.DefaultMargin;
            rightMargin: Constants.DefaultMargin;
        }
        highPrecision: true;
        onExponentialValueChanged: {
            if (useExponentialValue) {
                base.value = base.min + ((exponentialValue / 100) * (base.max - base.min))
            }
        }
        onValueChanged: {
            if (!useExponentialValue) {
                base.value = base.min + ((value / 100) * (base.max - base.min));
            }
        }
    }
    QtObject {
        id: d;
        function fixHandle() {
            var currentVal = base.value;
            // Set the value to something it isn't currently
            base.value = base.min;
            base.value = base.max;
            // Set it back to what it was
            base.value = currentVal;
        }
    }
}
