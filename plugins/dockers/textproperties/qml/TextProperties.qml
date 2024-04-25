/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.0
import QtQml.Models 2.1
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.krita.flake.text 1.0


Rectangle {
    id: root;
    color: sysPalette.window;
    anchors.fill: parent;

    SystemPalette {
        id: sysPalette;
        colorGroup: SystemPalette.Active
    }

    ListModel {
        id: characterPropertyList;
    }
    ListModel {
        id: paragraphPropertyList;
    }

    Component.onCompleted: fillPropertyModels();

    function fillPropertyModels() {
        for (var i = 0; i < characterPropertiesModel.count; i++) {
            characterPropertyList.append({"name": characterPropertiesModel.get(i).propertyName,
                                          "tooltip": characterPropertiesModel.get(i).toolTip,
                                          "searchTerms": characterPropertiesModel.get(i).searchTerms,
                                          "visible": characterPropertiesModel.get(i).visible,
                                          "item": characterPropertiesModel.get(i)});
        }
        for (var l = 0; l < paragraphPropertiesModel.count; l++) {
            paragraphPropertyList.append({"name": paragraphPropertiesModel.get(l).propertyName,
                                          "tooltip": paragraphPropertiesModel.get(l).toolTip,
                                          "searchTerms": paragraphPropertiesModel.get(l).searchTerms,
                                          "visible": paragraphPropertiesModel.get(l).visible,
                                          "item": paragraphPropertiesModel.get(l)});
        }
    }

    function setProperties() {
        for (var i = 0; i < characterPropertiesModel.count; i++) {
            characterPropertiesModel.get(i).propertiesUpdated();
        }
        for (var l = 0; l < paragraphPropertiesModel.count; l++) {
            paragraphPropertiesModel.get(l).propertiesUpdated();
        }
    }

    TabBar {
        id: tabs
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: parent.top;
        TabButton {
            text: i18nc("@title:tab", "Character")
        }
        TabButton {
            text: i18nc("@title:tab", "Paragraph")
        }
    }

    StackLayout {
        currentIndex: tabs.currentIndex;
        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: tabs.bottom;

        ColumnLayout {
            Frame {
                id: characterFrame;
                Layout.fillHeight: true;
                Layout.fillWidth: true;
                clip: true;
                padding: 0;

                ListView {
                    id: characterProperties

                    anchors.fill: parent;
                    ScrollBar.vertical: ScrollBar {
                    }

                    model: ObjectModel {
                        id: characterPropertiesModel;
                        FontSize {
                        }
                        FontFamily {
                            fontFamilyModel: fontFamiliesModel;
                        }
                        FontStyle {
                        }
                        LetterSpacing {
                        }
                        WordSpacing {
                        }
                        LineHeight {
                        }
                        LineBreak {
                        }
                        WordBreak {
                        }
                        TextTransform {
                        }
                        TextDecoration {
                        }
                        /*
                    OTLigatures {
                    }
                    OTPosition {
                    }
                    OTNumeric {
                    }
                    OTCaps {
                    }
                    OTEastAsian {
                    }*/
                        BaselineShift {
                        }
                        AlignmentBaseline {
                        }
                    }
                }
            }

            ComboBox {
                Layout.fillWidth: true;
                Layout.minimumHeight: implicitHeight;
                model: characterPropertyList;
                textRole: "name";
                displayText: i18nc("@label:listbox", "Add Property");
                onActivated: {
                    model.get(currentIndex).item.visible = true;
                }
            }
        }

        ColumnLayout {
            Frame {
                Layout.fillHeight: true;
                Layout.fillWidth: true;
                clip: true;
                padding: 0;

                ListView {
                    id: paragraphProperties;

                    clip: true;
                    anchors.fill: parent;
                    ScrollBar.vertical: ScrollBar {
                    }

                    model: ObjectModel {
                        id: paragraphPropertiesModel;
                        WritingMode {
                        }
                        Direction{
                        }
                        TextIndent{
                        }
                        TextAlign{
                        }
                        DominantBaseline {
                        }
                        /*
                        WhiteSpace {
                        }
                        UnderlinePosition {
                        }*/
                        HangingPunctuation {
                        }
                        TabSize {
                        }
                    }
                }
            }
            ComboBox {
                Layout.fillWidth: true;
                Layout.minimumHeight: implicitHeight;
                model: paragraphPropertyList;
                textRole: "name";
                displayText: i18nc("@label:listbox", "Add Property");
                onActivated: {
                    model.get(currentIndex).item.visible = true;
                }
            }
        }
    }
}
