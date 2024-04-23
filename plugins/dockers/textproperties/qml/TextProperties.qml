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

        ScrollView {
            background: Rectangle {
                color: sysPalette.alternateBase;
                border.color: sysPalette.base;
                border.width: 1;
            }
            clip: true;

            ListView {
                id: characterProperties
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

        ScrollView {
            background: Rectangle {
                color: sysPalette.alternateBase;
                border.color: sysPalette.base;
                border.width: 1;
            }
            clip: true;
            ListView {
                id: paragraphProperties
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
    }
}
