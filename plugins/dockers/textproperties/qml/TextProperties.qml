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

    property KoSvgTextPropertiesModel textPropertyModel : textPropertiesModel;

    onTextPropertyModelChanged: console.log("Writing Mode:" ,textPropertyModel.writingMode);

    SystemPalette {
        id: sysPalette;
        colorGroup: SystemPalette.Active
    }

    TabBar {
        id: tabs
        anchors.right: parent.right;
        anchors.left: parent.left;
        anchors.top: parent.top;
        TabButton {
            text: "Character"
        }
        TabButton {
            text: "Paragraph"
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

            ListView {
                id: characterProperties
                model: ObjectModel {
                    FontSize {
                        width: characterProperties.width;
                    }
                    FontFamily {
                        width: characterProperties.width;
                    }
                    FontStyle {
                        width: characterProperties.width;
                    }
                    LetterSpacing {
                        width: characterProperties.width;
                    }
                    WordSpacing {
                        width: characterProperties.width;
                    }
                    LineHeight {
                        width: characterProperties.width;
                    }
                    LineBreak {
                        width: characterProperties.width;
                    }
                    WordBreak {
                        width: characterProperties.width;
                    }
                    TextTransform {
                        width: characterProperties.width;
                    }
                    TextDecoration {
                        width: characterProperties.width;
                    }
                    OTLigatures {
                        width: characterProperties.width;
                    }
                    OTPosition {
                        width: characterProperties.width;
                    }
                    OTNumeric {
                        width: characterProperties.width;
                    }
                    OTCaps {
                        width: characterProperties.width;
                    }
                    OTEastAsian {
                        width: characterProperties.width;
                    }
                    BaselineShift {
                        width: characterProperties.width;
                    }
                    AlignmentBaseline {
                        width: characterProperties.width;
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
            ListView {
                id: paragraphProperties
                model: ObjectModel {
                    WritingMode {
                        width: parent.width;
                        writingMode: root.textPropertyModel.writingMode;
                    }
                    Direction{
                        width: parent.width;
                    }
                    TextIndent{
                        width: parent.width;
                    }
                    TextAlign{
                        width: parent.width;
                    }
                    DominantBaseline {
                        width: parent.width;
                    }
                    WhiteSpace {
                        width: parent.width;
                    }
                    UnderlinePosition {
                        width: parent.width;
                    }
                    HangingPunctuation {
                        width: parent.width;
                    }
                    TabSize {
                        width: parent.width;
                    }
                }
            }
        }
    }
}
