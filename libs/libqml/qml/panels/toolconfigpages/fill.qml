/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Column {
    id: base
    property bool fullView: true;
    height: childrenRect.height;
    spacing: Constants.DefaultMargin;

    Label {
        id: compositeModeListLabel
        visible: fullView;
        text: "Blending mode:"
        font: Settings.theme.font("panelSection");
    }
    ExpandingListView {
        id: compositeModeList
        visible: fullView;
        expandedHeight: Constants.GridHeight * 6;
        width: parent.width;

        property bool firstSet: false;
        onCurrentIndexChanged: {
            if (firstSet) { model.activateItem(currentIndex); }
            else { firstSet = true; }
        }
        model: compositeOpModel;
    }

    RangeInput {
        id: opacityInput;
        width: parent.width;
        placeholder: "Opacity";
        min: 0; max: 1; decimals: 2;
        value: compositeOpModel.opacity;
        onValueChanged: compositeOpModel.changePaintopValue("opacity", value);
        enabled: compositeOpModel.opacityEnabled;
    }

    RangeInput {
        id: thresholdInput;
        width: parent.width;
        placeholder: "Threshold";
        min: 0; max: 255; decimals: 0;
        value: 255;
        onValueChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetThreshold(value);
    }

    CheckBox {
        id: fillSelectionCheck;
        visible: fullView;
        width: parent.width;
        text: "Fill Selection";
        checked: false;
        onCheckedChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetFillSelection(checked);
    }

    CheckBox {
        id: limitToLayerCheck;
        visible: fullView;
        width: parent.width;
        text: "Limit to Layer";
        checked: false;
        onCheckedChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetSampleMerged(checked);
    }

    Component.onCompleted: compositeModeList.currentIndex = compositeOpModel.indexOf(compositeOpModel.currentCompositeOpID);
    Connections {
        target: compositeOpModel;
        onOpacityChanged: opacityInput.value = compositeOpModel.opacity;
        onCurrentCompositeOpIDChanged: {
            var newIndex = compositeOpModel.indexOf(compositeOpModel.currentCompositeOpID);
            if (compositeModeList.currentIndex !== newIndex) {
                compositeModeList.currentIndex = newIndex;
            }
        }
    }
}
