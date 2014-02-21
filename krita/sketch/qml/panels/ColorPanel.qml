/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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
import "../components"

Panel {
    id: base;
    name: "Color";
    panelColor: "#000000"

    actions: [
        ColorSwatch {
            id: swatch;
            height: parent.height;
            width: height;
            onChooseBGChanged: {
                if (toolManager.currentTool.toolId() === "KritaSelected/KisToolColorPicker") {
                    toolManager.currentTool.toForeground = !swatch.chooseBG;
                }
            }
        },
        Button {
            id: colorPicker;
            width: height;
            height: Constants.ToolbarButtonSize;
            color: "transparent";
            image: "../images/svg/icon-colorpicker.svg";
            textColor: "white";
            shadow: false;
            highlight: false;
            checked: false;
            onClicked: toolManager.requestToolChange("KritaSelected/KisToolColorPicker");
        },
        Item {
            width: base.width - Constants.DefaultMargin - (Constants.ToolbarButtonSize * 3);
            height: Constants.ToolbarButtonSize;
        },
        Button {
            id: showPeekGrid;
            visible: base.state === "peek";
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: colorSelectorPeek.visible ? "../images/svg/icon-palette.svg" : "../images/svg/icon-color_wheel.svg";
            textColor: "white";
            shadow: false;
            highlight: false;
            checked: false;
            onClicked: colorSelectorPeek.visible = !colorSelectorPeek.visible;
        }
    ]
    Connections {
        target: toolManager;
        onCurrentToolChanged: {
            if (toolManager.currentTool.toolId() === "KritaSelected/KisToolColorPicker") {
                toolManager.currentTool.toForeground = !swatch.chooseBG;
            }
        }
    }

    PaletteColorsModel {
        id: paletteColorsModel;
        view: sketchView.view;
        onColorChanged: {
            if (backgroundChanged) {
                swatch.bgColor = newColor;
            }
            else {
                swatch.fgColor = newColor;
            }
        }
    }
    PaletteModel {
        id: paletteModel;
    }

    peekContents: Item {
        anchors.fill: parent;
        Item {
            id: colorSelectorPeek;
            anchors {
                fill: parent;
                margins: Constants.DefaultMargin;
            }
            //height: parent.height;
            ColorSelectorItem {
                id: colorSelectorActualPeek;
                anchors.fill: parent;
                view: sketchView.view;
                changeBackground: swatch.chooseBG;
                shown: (base.state === "peek") && colorSelectorPeek.visible
                onColorChanged: {
                    if (backgroundChanged) {
                        swatch.bgColor = newColor;
                    }
                    else {
                        swatch.fgColor = newColor;
                    }
                    fullPaletteAlphaSlider.value = newAlpha * 100;
                }
            }
        }
        GridView {
            anchors {
                fill: parent
                margins: Constants.DefaultMargin;
            }
            model: paletteColorsModel;
            delegate: delegate;
            cellWidth: width / 2;
            cellHeight: Constants.GridHeight;
            visible: !colorSelectorPeek.visible;
            ScrollDecorator {
                flickableItem: parent;
            }
        }
    }

    fullContents: Item {
        anchors.fill: parent;
        Item {
            id: colorSelector;
            anchors {
                top: parent.top;
                left: parent.left;
                right: parent.right;
                margins: Constants.DefaultMargin;
            }
            height: parent.width;
            ColorSelectorItem {
                id: colorSelectorActual;
                anchors.fill: parent;
                view: sketchView.view;
                changeBackground: swatch.chooseBG;
                shown: (base.state === "full") && colorSelector.visible
                onColorChanged: {
                    if (backgroundChanged) {
                        swatch.bgColor = newColor;
                    }
                    else {
                        swatch.fgColor = newColor;
                    }
                    fullPaletteAlphaSlider.value = newAlpha * 100;
                }
            }
        }
        Slider {
            id: fullPaletteAlphaSlider;
            anchors {
                top: colorSelector.bottom;
                left: parent.left;
                right: parent.right;
                leftMargin: Constants.DefaultMargin;
                rightMargin: Constants.DefaultMargin;
            }
            value: 100;
            onValueChanged: {
                colorSelectorActual.setAlpha(value);
                colorSelectorActualPeek.setAlpha(value);
            }
        }
        ExpandingListView {
            id: fullPaletteList
            anchors {
                top: fullPaletteAlphaSlider.bottom;
                topMargin: Constants.DefaultMargin;
                left: parent.left;
                right: parent.right;
                leftMargin: Constants.DefaultMargin;
                rightMargin: Constants.DefaultMargin;
            }
            model: paletteModel
            onCurrentIndexChanged: {
                paletteModel.itemActivated(currentIndex);
                paletteColorsModel.colorSet = paletteModel.colorSet;
            }
        }
        GridView {
            anchors {
                top: fullPaletteList.bottom;
                topMargin: Constants.DefaultMargin;
                left: parent.left;
                right: parent.right;
                rightMargin: Constants.DefaultMargin;
                bottom: parent.bottom;
            }
            model: paletteColorsModel;
            delegate: delegate;
            clip: true;
            cellWidth: (width / 2) - 1;
            cellHeight: Constants.GridHeight;
            ScrollDecorator {
                flickableItem: parent;
            }
        }
    }

    Component {
        id: delegate;

        Item {
            width: Constants.GridWidth - Constants.DefaultMargin;
            height: Constants.GridHeight;
            Image {
                anchors {
                    fill: parent;
                    margins: Constants.DefaultMargin;
                }
                source: model.image;
                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        //Settings.currentColor = model.color;
                        paletteColorsModel.activateColor(index, swatch.chooseBG);
                    }
                }
            }
        }
    }
}
