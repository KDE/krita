/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 1.1
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Column {
    id: base
    property bool fullView: true;
    height: childrenRect.height;
    spacing: Constants.DefaultMargin;

    // === Blending Mode ===
    Label {
        visible: fullView;
        text: "Blending mode:"
        font: Settings.theme.font("panelSection");
    }
    ExpandingListView {
        id: compositeModeList
        visible: fullView;
        width: parent.width;
        expandedHeight: Constants.GridHeight * 6;

        property bool firstSet: false;
        onCurrentIndexChanged: {
            if (firstSet) { model.activateItem(currentIndex); }
            else { firstSet = true; }
        }

        model: compositeOpModel;
    }

    // === Brush Properties ===
    RangeCombo {
        id: sizeInput;
        width: parent.width;
        visible: compositeOpModel.sizeEnabled;
        placeholder: "Size";
        useExponentialValue: true;
        min: 1; max: 1000; decimals: 0;
        value: compositeOpModel.size;
        onValueChanged: compositeOpModel.changePaintopValue("size", value);
        model: ListModel {
            ListElement { value: 1 }
            ListElement { value: 2 }
            ListElement { value: 5 }
            ListElement { value: 10 }
            ListElement { value: 20 }
            ListElement { value: 50 }
            ListElement { value: 100 }
            ListElement { value: 200 }
            ListElement { value: 500 }
            ListElement { value: 1000 }
        }
    }

    RangeInput {
        id: opacityInput;
        width: parent.width;
        visible: compositeOpModel.opacityEnabled;
        placeholder: "Opacity";
        min: 0; max: 1; decimals: 2;
        value: compositeOpModel.opacity;
        onValueChanged: compositeOpModel.changePaintopValue("opacity", value);
    }

    RangeInput {
        id: flowInput;
        width: parent.width;
        placeholder: "Flow";
        min: 0; max: 1; decimals: 2;
        value: compositeOpModel.flow;
        onValueChanged: compositeOpModel.changePaintopValue("flow", value);
        visible: compositeOpModel.flowEnabled;
    }

    // === Smoothing ===
    Label {
        id: smoothingLabel
        visible: d.smoothingVisible;
        font: Settings.theme.font("panelSection");
        text: "Smoothing:";
    }

    ExpandingListView {
        id: smoothnessTypeList
        visible: d.smoothingVisible;
        width: parent.width;
        expandedHeight: Constants.GridHeight * 2
        model: ListModel {
            ListElement { text: "No smoothing" }
            ListElement { text: "Basic smoothing" }
            ListElement { text: "Weighted smoothing" }
        }
        currentIndex: (toolManager.currentTool && toolManager.currentTool.smoothingType) ? toolManager.currentTool.smoothingType : 1;
        onCurrentIndexChanged: if (toolManager.currentTool && toolManager.currentTool.smoothingType !== undefined && toolManager.currentTool.smoothingType !== currentIndex) toolManager.currentTool.smoothingType = currentIndex;
    }

    RangeInput {
        id: smoothnessQualitySlider;
        visible: d.smoothingVisible && smoothnessTypeList.currentIndex === 2
        width: parent.width;
        placeholder: "Distance";
        min: 3; max: 1000; decimals: 1;
        useExponentialValue: true;
        value: toolManager.currentTool.smoothnessQuality;
        onValueChanged: if (toolManager.currentTool && toolManager.currentTool.smoothnessQuality !== undefined && toolManager.currentTool.smoothnessQuality !== value) toolManager.currentTool.smoothnessQuality = value;
    }

    RangeInput {
        id: smoothnessFactorSlider;
        visible: d.smoothingVisible && smoothnessTypeList.currentIndex === 2
        width: parent.width;
        placeholder: "Stroke Ending"
        useExponentialValue: true;
        min: 0; max: 1; decimals: 2;
        value: toolManager.currentTool.smoothnessFactor;
        onValueChanged: if (toolManager.currentTool && toolManager.currentTool.smoothnessFactor !== undefined && toolManager.currentTool.smoothnessFactor !== value) toolManager.currentTool.smoothnessFactor = value;
    }

    CheckBox {
        id: smoothPressureCheck;
        visible: d.smoothingVisible && smoothnessTypeList.currentIndex === 2
        width: parent.width;
        text: "Smooth Pressure";
        checked: toolManager.currentTool ? toolManager.currentTool.smoothPressure : false;
        onCheckedChanged: if (toolManager.currentTool && toolManager.currentTool.smoothPressure !== undefined && toolManager.currentTool.smoothPressure !== checked) toolManager.currentTool.smoothPressure = checked;
    }

    // === Mirror ===
    Label {
        visible: fullView;
        font: Settings.theme.font("panelSection");
        height: Constants.GridHeight / 2;
        text: "Mirror:";
    }

    CheckBox {
        visible: fullView;
        width: parent.width;
        text: "Mirror Horizontally"
        checked: compositeOpModel.mirrorHorizontally;
        onCheckedChanged: compositeOpModel.mirrorHorizontally = checked;
    }

    CheckBox {
        visible: fullView;
        width: parent.width;
        text: "Mirror Vertically";
        checked: compositeOpModel.mirrorVertically;
        onCheckedChanged: compositeOpModel.mirrorVertically = checked;
    }

    // === Other Things ===
    Component.onCompleted: {
        compositeModeList.currentIndex = compositeOpModel.indexOf(compositeOpModel.currentCompositeOpID);

        if (toolManager.currentTool === null)
            return;

        smoothnessQualitySlider.value = toolManager.currentTool.smoothnessQuality;
        smoothnessFactorSlider.value = toolManager.currentTool.smoothnessFactor;
        smoothPressureCheck.checked = toolManager.currentTool.smoothPressure;
    }

    Connections {
        target: compositeOpModel;
        onSizeChanged: sizeInput.value = compositeOpModel.size;
        onOpacityChanged: opacityInput.value = compositeOpModel.opacity;
        onFlowChanged: flowInput.value = compositeOpModel.flow;
        onCurrentCompositeOpIDChanged: {
            var newIndex = compositeOpModel.indexOf(compositeOpModel.currentCompositeOpID);
            if (compositeModeList.currentIndex !== newIndex) {
                compositeModeList.currentIndex = newIndex;
            }
        }
    }

    Connections {
        target: toolManager;
        onCurrentToolChanged: {
            if (toolManager.currentTool === null || toolManager.currentTool.smoothnessQuality === undefined)
                return;
            smoothnessQualitySlider.value = toolManager.currentTool.smoothnessQuality;
            smoothnessFactorSlider.value = toolManager.currentTool.smoothnessFactor;
            smoothPressureCheck.checked = toolManager.currentTool.smoothPressure;
        }
    }

    QtObject {
        id: d;

        property bool smoothingVisible: base.fullView && toolManager.currentTool !== null && toolManager.currentTool.smoothingType !== undefined;
    }
}
