/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTPROPERTIESMODEL_H
#define KOSVGTEXTPROPERTIESMODEL_H

#include <QObject>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <lager/state.hpp>

#include "KoSvgText.h"
#include "KoSvgTextPropertyData.h"
#include "CssLengthPercentageModel.h"
#include "LineHeightModel.h"
#include "TextIndentModel.h"
#include "TabSizeModel.h"
#include "TextTransformModel.h"

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT KoSvgTextPropertiesModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CssLengthPercentageModel *fontSize READ fontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(LineHeightModel *lineHeight READ lineHeight NOTIFY lineHeightChanged)
    Q_PROPERTY(CssLengthPercentageModel *letterSpacing READ letterSpacing NOTIFY letterSpacingChanged)
    Q_PROPERTY(CssLengthPercentageModel *wordSpacing READ wordSpacing NOTIFY wordSpacingChanged)
    Q_PROPERTY(CssLengthPercentageModel *baselineShiftValue READ baselineShiftValue NOTIFY baselineShiftValueChanged)
    Q_PROPERTY(TextIndentModel *textIndent READ textIndent NOTIFY textIndentChanged)
    Q_PROPERTY(TabSizeModel *tabSize READ tabSize NOTIFY tabSizeChanged)
    Q_PROPERTY(TextTransformModel *textTransform READ textTransform NOTIFY textTransformChanged)
public:
    KoSvgTextPropertiesModel(lager::cursor<KoSvgTextPropertyData> _textData = lager::make_state(KoSvgTextPropertyData(), lager::automatic_tag{}));

    lager::cursor<KoSvgTextPropertyData> textData;
    lager::cursor<KoSvgTextProperties> commonProperties;
    lager::cursor<KoSvgText::CssLengthPercentage> fontSizeData;
    lager::cursor<KoSvgText::LineHeightInfo> lineHeightData;
    lager::cursor<KoSvgText::CssLengthPercentage> letterSpacingData;
    lager::cursor<KoSvgText::CssLengthPercentage> wordSpacingData;
    lager::cursor<KoSvgText::CssLengthPercentage> baselineShiftValueData;
    lager::cursor<KoSvgText::TextIndentInfo> textIndentData;
    lager::cursor<KoSvgText::TabSizeInfo> tabSizeData;
    lager::cursor<KoSvgText::TextTransformInfo> textTransformData;


    CssLengthPercentageModel fontSizeModel;
    LineHeightModel lineHeightModel;

    CssLengthPercentageModel letterSpacingModel;
    CssLengthPercentageModel wordSpacingModel;
    CssLengthPercentageModel baselineShiftValueModel;

    TextIndentModel textIndentModel;
    TabSizeModel tabSizeModel;
    TextTransformModel textTransformModel;

    // Whether a given property is set, unset or tristate.
    enum PropertyState {
        PropertyUnset,
        PropertySet,
        PropertyTriState
    };
    Q_ENUM(PropertyState)

    CssLengthPercentageModel *fontSize();
    LAGER_QT_CURSOR(PropertyState, fontSizeState);
    LineHeightModel *lineHeight();
    LAGER_QT_CURSOR(PropertyState, lineHeightState);

    CssLengthPercentageModel *letterSpacing();
    LAGER_QT_CURSOR(PropertyState, letterSpacingState);
    CssLengthPercentageModel *wordSpacing();
    LAGER_QT_CURSOR(PropertyState, wordSpacingState);
    CssLengthPercentageModel *baselineShiftValue();

    TextIndentModel *textIndent();
    LAGER_QT_CURSOR(PropertyState, textIndentState);
    TabSizeModel *tabSize();
    LAGER_QT_CURSOR(PropertyState, tabSizeState);
    TextTransformModel *textTransform();
    LAGER_QT_CURSOR(PropertyState, textTransformState);

    LAGER_QT_CURSOR(int, writingMode);
    LAGER_QT_CURSOR(PropertyState, writingModeState);
    LAGER_QT_CURSOR(int, direction);
    LAGER_QT_CURSOR(PropertyState, directionState);
    LAGER_QT_CURSOR(int, textAlignAll);
    LAGER_QT_CURSOR(PropertyState, textAlignAllState);
    LAGER_QT_CURSOR(int, textAlignLast);
    LAGER_QT_CURSOR(PropertyState, textAlignLastState);
    LAGER_QT_CURSOR(int, textAnchor);
    LAGER_QT_CURSOR(PropertyState, textAnchorState);

    LAGER_QT_CURSOR(int, fontWeight);
    LAGER_QT_CURSOR(PropertyState, fontWeightState);
    LAGER_QT_CURSOR(int, fontWidth);
    LAGER_QT_CURSOR(PropertyState, fontWidthState);

    // QFont::Style isn't exposed to qml.
    enum FontStyle {
        StyleNormal = QFont::StyleNormal,
        StyleItalic = QFont::StyleItalic,
        StyleOblique = QFont::StyleOblique
    };
    Q_ENUM(FontStyle)

    LAGER_QT_CURSOR(FontStyle, fontStyle);
    LAGER_QT_CURSOR(PropertyState, fontStyleState);
    LAGER_QT_CURSOR(bool, fontOpticalSizeLink);
    LAGER_QT_CURSOR(PropertyState, fontOpticalSizeLinkState);

    LAGER_QT_CURSOR(QStringList, fontFamilies);
    LAGER_QT_CURSOR(PropertyState, fontFamiliesState);

    LAGER_QT_CURSOR(bool, textDecorationUnderline);
    LAGER_QT_CURSOR(bool, textDecorationOverline);
    LAGER_QT_CURSOR(bool, textDecorationLineThrough);
    LAGER_QT_CURSOR(PropertyState, textDecorationLineState);

    LAGER_QT_CURSOR(int, textDecorationStyle);

    LAGER_QT_CURSOR(QColor, textDecorationColor);

    enum HangComma {
        NoHang,
        AllowHang,
        ForceHang
    };
    Q_ENUM(HangComma)
    LAGER_QT_CURSOR(bool, hangingPunctuationFirst);
    LAGER_QT_CURSOR(HangComma, hangingPunctuationComma);
    LAGER_QT_CURSOR(bool, hangingPunctuationLast);
    LAGER_QT_CURSOR(PropertyState, hangingPunctuationState);

    LAGER_QT_CURSOR(int, alignmentBaseline);
    LAGER_QT_CURSOR(PropertyState, alignmentBaselineState);
    LAGER_QT_CURSOR(int, dominantBaseline);
    LAGER_QT_CURSOR(PropertyState, dominantBaselineState);
    LAGER_QT_CURSOR(int, baselineShiftMode);
    LAGER_QT_CURSOR(PropertyState, baselineShiftState);

    LAGER_QT_CURSOR(int, wordBreak);
    LAGER_QT_CURSOR(PropertyState, wordBreakState);
    LAGER_QT_CURSOR(int, lineBreak);
    LAGER_QT_CURSOR(PropertyState, lineBreakState);

    /// We're selecting a span of text instead of the whole paragraph.
    LAGER_QT_CURSOR(bool, spanSelection);

Q_SIGNALS:
    void textPropertyChanged();
    void fontSizeChanged();
    void lineHeightChanged();

    void letterSpacingChanged();
    void wordSpacingChanged();
    void baselineShiftValueChanged();

    void textIndentChanged();
    void tabSizeChanged();
    void textTransformChanged();
};

#endif // KOSVGTEXTPROPERTIESMODEL_H
