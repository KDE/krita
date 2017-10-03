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

Page {
    Flickable {
        anchors.fill: parent;
        anchors.margins: Constants.DefaultMargin;

        contentWidth: width;
        contentHeight: contents.height;

        Column {
            id: contents;
            width: parent.width;

            TextField { placeholder: "Name"; text: "New Preset"; }

            Label { text: "Tool" }

            ListView {
                width: parent.width;
                height: Constants.GridHeight;
                orientation: ListView.Horizontal;
                spacing: Constants.DefaultMargin;
                clip: true;

                delegate: Button {
                    width: Constants.GridWidth;
                    height: Constants.GridHeight;
                    text: model.name;
                    textSize: Constants.SmallFontSize;
                    checked: ListView.isCurrentItem;

                    onClicked: ListView.view.currentIndex = index;
                }

                model: ListModel {
                    ListElement { name: "Round"; }
                    ListElement { name: "Square"; }
                    ListElement { name: "Smudge"; }
                    ListElement { name: "Fill"; }
                }
            }

            Label { text: "Brush Tip" }

            TextField { placeholder: "Diameter"; text: "10"; }
            TextField { placeholder: "Ratio"; text: "1.0"; }
            TextField { placeholder: "Fade"; text: "100%"; }
            TextField { placeholder: "Angle"; text: "0"; }

            Label { text: "Distribution" }

            TextField { placeholder: "Randomness"; text: "0"; }
            TextField { placeholder: "Density"; text: "100%"; }
            TextField { placeholder: "Spacing"; text: "0.10"; }
        }
    }
}
