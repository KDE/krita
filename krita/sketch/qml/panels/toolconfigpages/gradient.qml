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
        onOpacityChanged: opacityInput.value = compositeOpModel.opacity;
        onCurrentCompositeOpIDChanged: {
            var newIndex = compositeOpModel.indexOf(compositeOpModel.currentCompositeOpID);
            if (compositeModeList.currentIndex !== newIndex) {
                compositeModeList.currentIndex = newIndex;
            }
        }
    }

    Column {
        id: firstColumn
        anchors {
            top: fullView ? compositeModeList.bottom : compositeModeList.top;
            left: parent.left;
            leftMargin: Constants.DefaultMargin;
            right: parent.right;
            rightMargin: Constants.DefaultMargin;
        }
        height: childrenRect.height;
        RangeInput {
            id: opacityInput;
            width: parent.width;
            placeholder: "Opacity";
            min: 0; max: 1; decimals: 2;
            value: compositeOpModel.opacity;
            onValueChanged: compositeOpModel.changePaintopValue("opacity", value);
            enabled: compositeOpModel.opacityEnabled;
        }

        Item {
            width: parent.width;
            height: Constants.DefaultMargin;
        }
    }

    ExpandingListView {
        id: shapeList;
        anchors {
            top: firstColumn.bottom;
            left: parent.left;
            leftMargin: Constants.DefaultMargin;
            right: parent.right;
            rightMargin: Constants.DefaultMargin;
        }
        onCurrentIndexChanged: if (toolManager.currentTool && toolManager.currentTool.slotSetShape) toolManager.currentTool.slotSetShape(currentIndex);
        model: ListModel {
            ListElement {
                text: "Linear"
            }
            ListElement {
                text: "Bi-Linear";
            }
            ListElement {
                text: "Radial";
            }
            ListElement {
                text: "Square";
            }
            ListElement {
                text: "Conical";
            }
            ListElement {
                text: "Conical Symmetric";
            }
        }
    }

    Column {
        anchors {
            top: shapeList.bottom;
            left: parent.left;
            leftMargin: Constants.DefaultMargin;
            right: parent.right;
            rightMargin: Constants.DefaultMargin;
        }
        height: childrenRect.height;
        RangeInput {
            width: parent.width;
            placeholder: "Anti-alias";
            min: 0; max: 1; decimals: 3;
            value: 0.2;
            onValueChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetAntiAliasThreshold(value);
        }

        RangeInput {
            width: parent.width;
            placeholder: "Preview";
            min: 0; max: 100; decimals: 0;
            value: 75;
            onValueChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetPreviewOpacity(value);
        }

        CheckBox {
            width: parent.width;
            text: "Reverse";
            checked: false;
            onCheckedChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetReverse(checked);
        }
        CheckBox {
            width: parent.width;
            text: "Repeat";
            checked: false;
            onCheckedChanged: if (toolManager.currentTool) toolManager.currentTool.slotSetRepeat(checked);
        }
    }
}
