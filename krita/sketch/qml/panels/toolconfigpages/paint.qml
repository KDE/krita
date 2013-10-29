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
import "../../components"

Item {
    id: base
    property bool fullView: true;
    Label {
        id: compositeModeListLabel
        visible: fullView;
        height: fullView ? Constants.DefaultFontSize : 0;
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
            margins: Constants.DefaultMargin;
        }
        text: "Blending mode:"
    }
    ExpandingListView {
        id: compositeModeList
        visible: fullView;
        expandedHeight: Constants.GridHeight * 6;
        anchors {
            top: compositeModeListLabel.bottom;
            left: parent.left;
            right: parent.right;
            margins: Constants.DefaultMargin;
        }
        property bool firstSet: false;
        onCurrentIndexChanged: {
            if (firstSet) { model.activateItem(currentIndex); }
            else { firstSet = true; }
        }
        model: compositeOpModel;
    }
    Component.onCompleted: compositeModeList.currentIndex = compositeOpModel.indexOf(compositeOpModel.currentCompositeOpID);
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
    Column {
        anchors {
            top: fullView ? compositeModeList.bottom : compositeModeList.top;
            left: parent.left;
            leftMargin: Constants.DefaultMargin;
            right: parent.right;
            rightMargin: Constants.DefaultMargin;
        }
        height: childrenRect.height;

        RangeInput {
            id: sizeInput;
            width: parent.width;
            visible: compositeOpModel.sizeEnabled;
            placeholder: "Size";
            useExponentialValue: true;
            min: 1; max: 1000; decimals: 0;
            value: compositeOpModel.size;
            onValueChanged: compositeOpModel.changePaintopValue("size", value);
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

        Column {
            visible: fullView && toolManager.currentTool !== null && toolManager.currentTool.smoothingType !== undefined;
            height: !visible ? 0 : (smoothnessTypeList.currentIndex < 2 ? smoothingLabel.height + smoothnessTypeList.height : smoothingLabel.height + smoothnessTypeList.height + smoothnessQualitySlider.height + smoothnessFactorSlider.height + smoothPressureCheck.height)
            width: parent.width;
            Label {
                id: smoothingLabel
                horizontalAlignment: Text.AlignLeft;
                font.pixelSize: Constants.DefaultFontSize;
                font.bold: true;
                height: visible ? Constants.GridHeight / 2 : 0;
                text: "Smoothing:";
            }
            ExpandingListView {
                id: smoothnessTypeList
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
                visible: smoothnessTypeList.currentIndex === 2
                height: visible ? childrenRect.height : 0;
                Behavior on height { NumberAnimation { duration: 150; } }
                width: parent.width;
                placeholder: "Distance";
                min: 3; max: 1000; decimals: 1;
                useExponentialValue: true;
                value: toolManager.currentTool.smoothnessQuality;
                onValueChanged: if (toolManager.currentTool && toolManager.currentTool.smoothnessQuality !== undefined && toolManager.currentTool.smoothnessQuality !== value) toolManager.currentTool.smoothnessQuality = value;
            }
            RangeInput {
                id: smoothnessFactorSlider;
                visible: smoothnessTypeList.currentIndex === 2
                height: visible ? childrenRect.height : 0;
                Behavior on height { NumberAnimation { duration: 150; } }
                width: parent.width;
                placeholder: "Stroke Ending"
                useExponentialValue: true;
                min: 0; max: 1; decimals: 2;
                value: toolManager.currentTool.smoothnessFactor;
                onValueChanged: if (toolManager.currentTool && toolManager.currentTool.smoothnessFactor !== undefined && toolManager.currentTool.smoothnessFactor !== value) toolManager.currentTool.smoothnessFactor = value;
            }
            CheckBox {
                id: smoothPressureCheck;
                visible: smoothnessTypeList.currentIndex === 2
                height: visible ? Constants.DefaultFontSize + Constants.DefaultMargin * 4 : 0;
                Behavior on height { NumberAnimation { duration: 150; } }
                width: parent.width;
                text: "Smooth Pressure";
                checked: toolManager.currentTool ? toolManager.currentTool.smoothPressure : false;
                onCheckedChanged: if (toolManager.currentTool && toolManager.currentTool.smoothPressure !== undefined && toolManager.currentTool.smoothPressure !== checked) toolManager.currentTool.smoothPressure = checked;
            }
            Component.onCompleted: {
                if (toolManager.currentTool === null)
                    return;
                smoothnessQualitySlider.value = toolManager.currentTool.smoothnessQuality;
                smoothnessFactorSlider.value = toolManager.currentTool.smoothnessFactor;
                smoothPressureCheck.checked = toolManager.currentTool.smoothPressure;
            }
            Connections {
                target: toolManager;
                onCurrentToolChanged: {
                    if (toolManager.currentTool.smoothnessQuality === undefined)
                        return;
                    smoothnessQualitySlider.value = toolManager.currentTool.smoothnessQuality;
                    smoothnessFactorSlider.value = toolManager.currentTool.smoothnessFactor;
                    smoothPressureCheck.checked = toolManager.currentTool.smoothPressure;
                }
            }
        }
// No Assistant for the first release
//         RangeInput {
//             visible: fullView;
//             width: parent.width;
//             placeholder: "Assistant";
//             min: 0; max: 1000; decimals: 0;
//             value: 1000;
//             onValueChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetMagnetism(value);
//         }

        Label {
            visible: fullView;
            width: parent.width;
            horizontalAlignment: Text.AlignLeft;
            font.pixelSize: Constants.LargeFontSize;
            font.bold: true;
            height: Constants.GridHeight / 2;
            text: "Mirror:";
        }

        Item {
            width: childrenRect.width;
            height: childrenRect.height;
            anchors.horizontalCenter: parent.horizontalCenter;
            Button {
                id: mirrorVertical;
                image: "../../images/svg/icon-mirror_v-black.svg"
                width: Constants.GridWidth * 2 / 5;
                height: width;
                color: "transparent";
                shadow: false;
                highlight: false;
                checked: compositeOpModel.mirrorVertically;
                onClicked: compositeOpModel.mirrorVertically = !compositeOpModel.mirrorVertically;
            }
            Button {
                id: mirrorHorizontal;
                anchors.left: mirrorVertical.right;
                image: "../../images/svg/icon-mirror_h-black.svg"
                width: Constants.GridWidth * 2 / 5;
                height: width;
                color: "transparent";
                shadow: false;
                highlight: false;
                checked: compositeOpModel.mirrorHorizontally;
                onClicked: compositeOpModel.mirrorHorizontally = !compositeOpModel.mirrorHorizontally;
            }
            Button {
                id: mirrorCenter;
                anchors.left: mirrorHorizontal.right;
                image: "../../images/svg/icon-mirror_c-black.svg"
                width: Constants.GridWidth * 2 / 5;
                height: width;
                color: "transparent";
                shadow: false;
                highlight: false;
                onClicked: compositeOpModel.setMirrorCenter();
            }
        }
    }
}
