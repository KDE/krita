/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0
import org.krita.sketch.components 1.0

Page {
    id:base;

    signal finished(variant options);

    Rectangle {
        anchors.fill: parent;
        color: Settings.theme.color("pages/customImagePage/background");
    }

    Header {
        id: header;
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        text: "Custom Image"

        leftArea: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;
            image: Settings.theme.icon("back");
            onClicked: pageStack.pop();
        }

        rightArea: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;
            image: Settings.theme.icon("apply");
            onClicked: {
                var options = {
                    name: nameField.text,
                    width: widthField.text,
                    height: heightField.text,
                    resolution: resolutionField.text,
                    colorModelId: colorModelModel.id(colorModel.currentIndex),
                    colorDepthId: colorDepthModel.id(colorDepth.currentIndex),
                    colorProfileId: colorProfileModel.id(colorProfile.currentIndex),
                    "backgroundColor": backgroundColor.value,
                    "backgroundOpacity": backgroundOpacity.value / 100,
                }
                base.finished(options);
            }
        }
    }

    Column {
        anchors {
            top: header.bottom;
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
            topMargin: Constants.GridHeight * 0.35;
            leftMargin: Constants.GridWidth * 0.25;
            rightMargin: Constants.GridWidth * 0.25;
        }
        spacing: Constants.DefaultMargin;

        TextField {
            id: nameField;

            height: Constants.GridHeight * 0.75;
            width: parent.width + Constants.DefaultMargin * 2;
            x: -Constants.DefaultMargin;

            placeholder: "Name"
            nextFocus: widthField;

            // Focus handling
            focus: true;
            KeyNavigation.tab: widthField.textInput;
            KeyNavigation.backtab: backgroundOpacity;
            KeyNavigation.priority: KeyNavigation.BeforeItem;
        }

        Rectangle {
            color: Settings.theme.color("pages/customImagePage/groupBox");

            height: childrenRect.height;
            width: parent.width;
            radius: Constants.DefaultMargin;

            Column {
                spacing: Constants.DefaultMargin;
                width: parent.width;

                Label {
                    height: Constants.GridHeight * 0.75;
                    width: parent.width - Constants.DefaultMargin * 2;
                    x: Constants.DefaultMargin;

                    text: "Image Size"
                }

                Row {
                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    TextField {
                        id: widthField;

                        width: parent.width / 2;
                        height: parent.height;

                        placeholder: "Width";
                        validator: IntValidator{bottom: 0; top: 100000000;}
                        numeric: true;
                        nextFocus: heightField;

                        background: Settings.theme.color("pages/customImagePage/controls/background");
                        border.color: Settings.theme.color("pages/customImagePage/controls/border");
                        border.width: 1;

                        // Focus handling
                        KeyNavigation.tab: heightField.textInput;
                        KeyNavigation.backtab: nameField.textInput;
                        KeyNavigation.priority: KeyNavigation.BeforeItem;

                        Component.onCompleted: text = Settings.customImageSettings.readProperty("Width"); //Krita.Window.width;
                    }
                    TextField {
                        id: heightField;

                        width: parent.width / 2;
                        height: parent.height;

                        placeholder: "Height"
                        validator: IntValidator{bottom: 0; top: 100000000;}
                        numeric: true;
                        nextFocus: resolutionField;

                        background: Settings.theme.color("pages/customImagePage/controls/background");
                        border.color: Settings.theme.color("pages/customImagePage/controls/border");
                        border.width: 1;

                        // Focus handling
                        KeyNavigation.tab: resolutionField.textInput;
                        KeyNavigation.backtab: widthField.textInput;
                        KeyNavigation.priority: KeyNavigation.BeforeItem;

                        Component.onCompleted: text = Settings.customImageSettings.readProperty("Height"); //Krita.Window.height;
                    }
                }
                TextField {
                    id: resolutionField;

                    height: Constants.GridHeight * 0.75;

                    background: Settings.theme.color("pages/customImagePage/controls/background");
                    border.color: Settings.theme.color("pages/customImagePage/controls/border");
                    border.width: 1;

                    placeholder: "Resolution"
                    text: "72";
                    validator: IntValidator{bottom: 0; top: 600;}
                    numeric: true;

                    // Focus handling
                    KeyNavigation.tab: colorModel;
                    KeyNavigation.backtab: heightField.textInput;
                    KeyNavigation.priority: KeyNavigation.BeforeItem;

                    Component.onCompleted: text = Settings.customImageSettings.readProperty("Resolution");
                }
            }
        }
        Rectangle {
            color: Settings.theme.color("pages/customImagePage/groupBox");

            height: childrenRect.height;
            width: parent.width;
            radius: Constants.DefaultMargin;

            Column {
                x: Constants.DefaultMargin;
                width: parent.width - Constants.DefaultMargin * 2;

                Label {
                    height: Constants.GridHeight * 0.75;
                    text: "Color"
                }

                ExpandingListView {
                    id: colorModel;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 3;

                    model: ColorModelModel { id: colorModelModel; }
                    Component.onCompleted: currentIndex = colorModelModel.indexOf(Settings.customImageSettings.readProperty("ColorModel"));

                    // Focus handling
                    KeyNavigation.tab: colorDepth;
                    KeyNavigation.backtab: resolutionField.textInput;
                    KeyNavigation.priority: KeyNavigation.BeforeItem;
                }

                ExpandingListView {
                    id: colorDepth;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 3;

                    model: ColorDepthModel { id: colorDepthModel; colorModelId: colorModelModel.id(colorModel.currentIndex); }
                    Component.onCompleted: currentIndex = colorDepthModel.indexOf(Settings.customImageSettings.readProperty("ColorDepth"));

                    // Focus handling
                    KeyNavigation.tab: colorProfile;
                    KeyNavigation.backtab: colorModel;
                    KeyNavigation.priority: KeyNavigation.BeforeItem;
                }

                ExpandingListView {
                    id: colorProfile;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 3;

                    currentIndex: colorProfileModel.defaultProfile;

                    model: ColorProfileModel {
                        id: colorProfileModel;
                        colorModelId: colorModelModel.id(colorModel.currentIndex);
                        colorDepthId: colorDepthModel.id(colorDepth.currentIndex);
                    }

                    // Focus handling
                    KeyNavigation.tab: backgroundColor;
                    KeyNavigation.backtab: colorDepth;
                    KeyNavigation.priority: KeyNavigation.BeforeItem;
                }
            }
        }

        Rectangle {
            color: Settings.theme.color("pages/customImagePage/groupBox");

            height: childrenRect.height;
            width: parent.width;
            radius: Constants.DefaultMargin;

            Column {
                x: Constants.DefaultMargin;
                width: parent.width - Constants.DefaultMargin * 2;

                Label {
                    height: Constants.GridHeight * 0.75;
                    text: "Background"
                }

                ExpandingListView {
                    id: backgroundColor;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 2;

                    model: ListModel {
                        ListElement { text: "White"; r: 1.0; g: 1.0; b: 1.0; }
                        ListElement { text: "Black"; r: 0.0; g: 0.0; b: 0.0; }
                        ListElement { text: "Gray"; r: 0.5; g: 0.5; b: 0.5; }
                        ListElement { text: "Red"; r: 1.0; g: 0.0; b: 0.0; }
                        ListElement { text: "Green"; r: 0.0; g: 1.0; b: 0.0; }
                        ListElement { text: "Blue"; r: 0.0; g: 0.0; b: 1.0; }
                        ListElement { text: "Cyan"; r: 0.0; g: 1.0; b: 1.0; }
                        ListElement { text: "Magenta"; r: 1.0; g: 0.0; b: 1.0; }
                        ListElement { text: "Yellow"; r: 1.0; g: 1.0; b: 0.0; }
                    }

                    property color value: Qt.rgba(0.0, 0.0, 0.0, 0.0);

                    onCurrentIndexChanged: {
                        var item = model.get(currentIndex);
                        value = Qt.rgba(item.r, item.g, item.b, 1.0);
                    }

                    // Focus handling
                    KeyNavigation.tab: backgroundOpacity;
                    KeyNavigation.backtab: colorProfile;
                    KeyNavigation.priority: KeyNavigation.BeforeItem;
                }

                RangeInput {
                    id: backgroundOpacity;

                    width: parent.width + 2 * Constants.DefaultMargin;
                    x: -Constants.DefaultMargin;
                    height: Constants.GridHeight * 1.5;

                    background: Settings.theme.color("pages/customImagePage/controls/background");
                    border.color: Settings.theme.color("pages/customImagePage/controls/border");
                    border.width: 1;

                    min: 0;
                    max: 100;
                    decimals: 0;
                    value: 100;
                    placeholder: "Opacity";

                    // Focus handling
                    KeyNavigation.tab: nameField.textInput;
                    KeyNavigation.backtab: backgroundColor;
                    KeyNavigation.priority: KeyNavigation.BeforeItem;
                }
            }
        }
    }
}
