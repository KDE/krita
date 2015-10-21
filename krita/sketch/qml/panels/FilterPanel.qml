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

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Panel {
    id: base;
    name: "Filter";
    colorSet: "filter";

    actions: [
        Button {
            id: applyButton;
            width: height;
            height: Constants.ToolbarButtonSize;
            image: Settings.theme.icon("apply")
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
            image: filtersCategoryModel.previewEnabled ? Settings.theme.icon("visible_on") : Settings.theme.icon("visible_off");
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
                    configLoader.source = "filterconfigpages/" + model.filterID(currentIndex) + ".qml";
                    if (configLoader.item && typeof(configLoader.item.configuration) !== 'undefined') {
                        configLoader.item.configuration = model.configuration(currentIndex);
                    }
                }
                else {
                    configLoader.source = "filterconfigpages/nothing-to-configure.qml";
                }
                filtersCategoryModel.filterSelected(currentIndex);
            }
        }
        Item {
            width: parent.width;
            height: parent.height - Constants.DefaultMargin - fullCategoryList.height - fullFilters.height;
            clip: true;
            Flickable {
                id: configNeeded;
                anchors.fill: parent;
                contentWidth: width;
                contentHeight: configLoader.height > 0 ? configLoader.height : 1;
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
}
