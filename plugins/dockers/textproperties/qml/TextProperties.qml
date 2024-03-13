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

            ListView {
                id: characterProperties
                model: ObjectModel {
                    FontSize {
                        width: characterProperties.width;

                        fontSize: textPropertiesModel.fontSize.value;
                        onFontSizeChanged: {textPropertiesModel.fontSize.value = fontSize;}

                    }
                    FontFamily {
                        width: characterProperties.width;
                        fontFamilies: textPropertiesModel.fontFamilies;
                        onFontFamiliesChanged: textPropertiesModel.fontFamilies = fontFamilies;
                        fontFamilyModel: fontFamiliesModel;
                    }
                    FontStyle {
                        width: characterProperties.width;
                        fontWeight: textPropertiesModel.fontWeight;
                        onFontWeightChanged: {textPropertiesModel.fontWeight = fontWeight;}
                        fontWidth: textPropertiesModel.fontWidth;
                        onFontWidthChanged: textPropertiesModel.fontWidth = fontWidth;
                        fontSlant: textPropertiesModel.fontStyle;
                        onFontSlantChanged: textPropertiesModel.fontStyle = fontSlant;
                        fontOptical: textPropertiesModel.fontOpticalSizeLink;
                        onFontOpticalChanged: textPropertiesModel.fontOpticalSizeLink = fontOptical;
                    }
                    LetterSpacing {
                        width: characterProperties.width;
                        letterSpacing: textPropertiesModel.letterSpacing.value;
                        onLetterSpacingChanged: textPropertiesModel.letterSpacing.value = letterSpacing;
                    }
                    WordSpacing {
                        width: characterProperties.width;
                        wordSpacing: textPropertiesModel.wordSpacing.value;
                        onWordSpacingChanged: textPropertiesModel.wordSpacing.value = wordSpacing;
                    }
                    LineHeight {
                        width: characterProperties.width;
                        isNormal: textPropertiesModel.lineHeight.isNormal;
                        onIsNormalChanged: textPropertiesModel.lineHeight.isNormal = isNormal;
                        lineHeight: textPropertiesModel.lineHeight.value;
                        onLineHeightChanged: textPropertiesModel.lineHeight.value = lineHeight;
                        lineHeightUnit: textPropertiesModel.lineHeight.unit;
                        onLineHeightUnitChanged: textPropertiesModel.lineHeight.unit = lineHeightUnit;
                    }
                    LineBreak {
                        width: characterProperties.width;
                        breakType: textPropertiesModel.lineBreak;
                        onBreakTypeChanged: textPropertiesModel.lineBreak = breakType;
                    }
                    WordBreak {
                        width: characterProperties.width;
                        breakType: textPropertiesModel.wordBreak;
                        onBreakTypeChanged: textPropertiesModel.wordBreak = breakType;
                    }
                    TextTransform {
                        width: characterProperties.width;
                        fullWidth: textPropertiesModel.textTransform.fullWidth;
                        onFullWidthChanged: textPropertiesModel.textTransform.fullWidth = fullWidth;
                        fullSizeKana: textPropertiesModel.textTransform.fullSizeKana;
                        onFullSizeKanaChanged: textPropertiesModel.textTransform.fullSizeKana = fullSizeKana;
                        capitals: textPropertiesModel.textTransform.capitals;
                        onCapitalsChanged: textPropertiesModel.textTransform.capitals = capitals;
                    }
                    TextDecoration {
                        width: characterProperties.width;
                        underline: textPropertiesModel.textDecorationUnderline;
                        onUnderlineChanged: textPropertiesModel.textDecorationUnderline = underline;
                        overline: textPropertiesModel.textDecorationOverline;
                        onOverlineChanged: textPropertiesModel.textDecorationOverline = overline;
                        linethrough: textPropertiesModel.textDecorationLineThrough;
                        onLinethroughChanged: textPropertiesModel.textDecorationLineThrough = linethrough;
                        lineStyle: textPropertiesModel.textDecorationStyle;
                        onLineStyleChanged: textPropertiesModel.textDecorationStyle = lineStyle;
                        lineColor: textPropertiesModel.textDecorationColor;
                    }
                    /*
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
                    }*/
                    BaselineShift {
                        width: characterProperties.width;
                        baselineShiftValue: textPropertiesModel.baselineShiftValue.value;
                        onBaselineShiftValueChanged: textPropertiesModel.baselineShiftValue.value = baselineShiftValue;
                        baselineShiftMode: textPropertiesModel.baselineShiftMode;
                        onBaselineShiftModeChanged: textPropertiesModel.baselineShiftMode = baselineShiftMode;
                    }
                    AlignmentBaseline {
                        width: characterProperties.width;
                        baselineSelection: textPropertiesModel.alignmentBaseline;
                        onBaselineSelectionChanged: textPropertiesModel.alignmentBaseline = baselineSelection;
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
                        writingMode: textPropertiesModel.writingMode;
                        onWritingModeChanged: {textPropertiesModel.writingMode = writingMode;}
                    }
                    Direction{
                        width: parent.width;
                        direction: textPropertiesModel.direction;
                        onDirectionChanged: textPropertiesModel.direction = direction;
                    }
                    TextIndent{
                        width: parent.width;
                        textIndentValue: textPropertiesModel.textIndent.length.value;
                        onTextIndentValueChanged: textPropertiesModel.textIndent.length.value = textIndentValue;
                        hanging: textPropertiesModel.textIndent.hanging;
                        onHangingChanged: textPropertiesModel.textIndent.hanging = hanging;
                        eachLine: textPropertiesModel.textIndent.eachLine;
                        onEachLineChanged: textPropertiesModel.textIndent.eachLine = eachLine;
                    }
                    TextAlign{
                        width: parent.width;
                        textAlignAll: textPropertiesModel.textAlignAll;
                        onTextAlignAllChanged: textPropertiesModel.textAlignAll = textAlignAll;
                        textAlignLast: textPropertiesModel.textAlignLast;
                        onTextAlignLastChanged: textPropertiesModel.textAlignLast = textAlignLast;
                        textAnchor: textPropertiesModel.textAnchor;
                        onTextAnchorChanged: textPropertiesModel.textAnchor = textAnchor;
                    }
                    DominantBaseline {
                        width: parent.width;
                        baselineSelection: textPropertiesModel.dominantBaseline;
                        onBaselineSelectionChanged: textPropertiesModel.dominantBaseline = baselineSelection;
                    }
                    /*
                    WhiteSpace {
                        width: parent.width;
                    }
                    UnderlinePosition {
                        width: parent.width;
                    }*/
                    HangingPunctuation {
                        width: parent.width;
                        hangStart: textPropertiesModel.hangingPunctuationFirst;
                        onHangStartChanged: textPropertiesModel.hangingPunctuationFirst = hangStart;
                        hangEnd: textPropertiesModel.hangingPunctuationLast;
                        onHangEndChanged: textPropertiesModel.hangingPunctuationLast = hangEnd;
                        hangComma: textPropertiesModel.hangingPunctuationComma;
                        onHangCommaChanged: textPropertiesModel.hangingPunctuationComma = hangComma;
                    }
                    TabSize {
                        width: parent.width;
                        tabSize: textPropertiesModel.tabSize.value;
                        onTabSizeChanged: textPropertiesModel.tabSize.value = tabSize;
                        tabSizeUnit: textPropertiesModel.tabSize.unit;
                        onTabSizeUnitChanged: textPropertiesModel.tabSize.unit = tabSizeUnit;
                    }
                }
            }
        }
    }
}
