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
    name: "Filter";
    panelColor: "#000000";

    actions: [
        Button {
            id: applyButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: "../images/svg/icon-apply.svg"
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: {
                if (base.state === "full") {
                    fullFilters.model.activateFilter(fullFilters.currentIndex);
                }
                else if (base.state === "peek") {
                    peekFilters.model.activateFilter(peekFilters.currentIndex);
                }
            }
        },
        Item {
            width: base.width - Constants.DefaultMargin - (Constants.ToolbarButtonSize * 2)
            height: Constants.ToolbarButtonSize;
        },
        Button {
            id: toggleShowPreviewButton;
            width: height;
            height: Constants.ToolbarButtonSize
            color: "transparent";
            image: filtersCategoryModel.previewEnabled ? "../images/svg/icon-visible_on.svg" : "../images/svg/icon-visible_off.svg";
            textColor: "white";
            shadow: false;
            highlight: false;
            onClicked: filtersCategoryModel.previewEnabled = !filtersCategoryModel.previewEnabled;
        }
    ]

    FiltersCategoryModel {
        id: filtersCategoryModel;
        view: sketchView.view;
    }

    peekContents: Column {
        anchors {
            fill: parent;
            margins: Constants.DefaultMargin;
        }
        spacing: Constants.DefaultMargin;
        ExpandingListView {
            id: categoryList;
            width: parent.width;
            model: filtersCategoryModel;
            onModelChanged: currentIndex = 0;
            onCurrentIndexChanged: {
                fullCategoryList.currentIndex = currentIndex;
                model.activateItem(currentIndex)
            }
        }
        ExpandingListView {
            id: peekFilters;
            width: parent.width;
            model: filtersCategoryModel.filterModel;
            onCurrentIndexChanged: {
                filtersCategoryModel.filterSelected(currentIndex);
            }
        }
    }

    fullContents: Column {
        anchors {
            fill: parent;
            margins: Constants.DefaultMargin;
        }
        spacing: Constants.DefaultMargin;
        ExpandingListView {
            id: fullCategoryList;
            width: parent.width;
            model: filtersCategoryModel;
            onCurrentIndexChanged: {
                if (categoryList.currentIndex !== currentIndex) {
                    categoryList.currentIndex = currentIndex;
                }
            }
        }
        ExpandingListView {
            id: fullFilters;
            width: parent.width;
            model: filtersCategoryModel.filterModel;
            function applyConfiguration(configuration) {
                model.setConfiguration(fullFilters.currentIndex, configuration);
            }
            onModelChanged: currentIndex = 0;
            onCurrentIndexChanged: {
                if (model.filterRequiresConfiguration(currentIndex)) {
                    noConfigNeeded.visible = false;
                    configNeeded.visible = true;
                    configLoader.source = "filterconfigpages/" + model.filterID(currentIndex) + ".qml";
                    if (typeof(configLoader.item.configuration) !== 'undefined') {
                        configLoader.item.configuration = model.configuration(currentIndex);
                    }
                }
                else {
                    noConfigNeeded.visible = true;
                    configNeeded.visible = false;
                }
                filtersCategoryModel.filterSelected(currentIndex);
            }
        }
        Item {
            id: noConfigNeeded;
            width: parent.width;
            height: parent.width;
            Column {
                anchors.fill: parent;
                Item {
                    width: parent.width;
                    height: Constants.GridHeight;
                }
                Text {
                    width: parent.width;
                    font.pixelSize: Constants.DefaultFontSize;
                    color: Constants.Theme.TextColor;
                    font.family: "Source Sans Pro"
                    wrapMode: Text.WordWrap;
                    horizontalAlignment: Text.AlignHCenter;
                    text: "This filter requires no configuration. Click below to apply it.";
                }
                Item {
                    width: parent.width;
                    height: Constants.GridHeight / 2;
                }
                Button {
                    width: height;
                    height: Constants.GridHeight
                    anchors.horizontalCenter: parent.horizontalCenter;
                    color: "transparent";
                    image: "../images/svg/icon-apply.svg"
                    textColor: "white";
                    shadow: false;
                    highlight: false;
                    onClicked: fullFilters.model.activateFilter(fullFilters.currentIndex);
                }
            }
        }
        Item {
            id: configNeeded;
            width: parent.width;
            height: childrenRect.height > 0 ? childrenRect.height : 1;
            MouseArea {
                anchors.fill: parent;
                hoverEnabled: true;
                onContainsMouseChanged: configLoader.focus = containsMouse;
            }
            Loader {
                id: configLoader;
                width: parent.width;
                height: item ? item.height : 1;
            }
        }
    }
}