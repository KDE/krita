/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
