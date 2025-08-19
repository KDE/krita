/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <KisLager.h>
#include "KoSvgTextPropertiesModel.h"
#include "KoSvgTextProperties.h"
#include <lager/constant.hpp>
#include <QDebug>

using kislager::lenses::variant_to;

namespace {

auto createTextProperty = [](KoSvgTextProperties::PropertyId propId) { return lager::lenses::getset(
                [propId](const KoSvgTextPropertyData &value) -> QVariant {
        QVariant defaultVar = value.inheritedProperties.propertyOrDefault(propId);
        return value.commonProperties.property(propId, defaultVar);
    },
    [propId](KoSvgTextPropertyData value, const QVariant &variant) -> KoSvgTextPropertyData {
        value.commonProperties.setProperty(propId, variant);
        return value;
    }
    );
};

auto propertyModifyState = [](KoSvgTextProperties::PropertyId propId) { return lager::lenses::getset(
                [propId](const KoSvgTextPropertyData &value) -> KoSvgTextPropertiesModel::PropertyState {
        if (value.commonProperties.hasProperty(propId)) {
            return KoSvgTextPropertiesModel::PropertySet;
        } else if (value.tristate.contains(propId)) {
            return KoSvgTextPropertiesModel::PropertyTriState;
        } else if (value.inheritedProperties.hasProperty(propId) && value.inheritedProperties.propertyIsInheritable(propId)) {
            return KoSvgTextPropertiesModel::PropertyInherited;
        }
        return KoSvgTextPropertiesModel::PropertyUnset;
    },
    [propId](KoSvgTextPropertyData value, const KoSvgTextPropertiesModel::PropertyState &state) -> KoSvgTextPropertyData {
        if (state == KoSvgTextPropertiesModel::PropertySet) {
            value.commonProperties.setProperty(propId, value.inheritedProperties.propertyOrDefault(propId));
        } else {
            // Because tristate represents properties that are set but mixed, there's no value in being able to set them from the UX.
            // so if we receive such a signal, it is incorrect.
            if (state == KoSvgTextPropertiesModel::PropertyTriState || state == KoSvgTextPropertiesModel::PropertyInherited) {
                qWarning() << "Receiving request to set property tristate or inherited, this will unset the property instead";
            }
            value.commonProperties.removeProperty(propId);
        }
        value.tristate.remove(propId);
        return value;
    }
    );
};

auto simplifiedAutoLengthPropertyImpl = lager::lenses::getset(
            [](const KoSvgText::AutoLengthPercentage &length) -> KoSvgText::CssLengthPercentage {
                return length.isAuto? KoSvgText::CssLengthPercentage(): length.length;
            },
            [](KoSvgText::AutoLengthPercentage length, const KoSvgText::CssLengthPercentage &val){
                length = val; return length;
            }
        );

auto simplifiedAutoLengthProperty = variant_to<KoSvgText::AutoLengthPercentage> | simplifiedAutoLengthPropertyImpl;

// Used for font-kerning.
auto autoValueToBoolImpl = lager::lenses::getset(
            [](const KoSvgText::AutoValue &value) -> bool {
                return value.isAuto;
            },
            [](KoSvgText::AutoValue value, const bool &val){
                value.isAuto = val;
                if (!val) {
                    value.customValue = 0;
                }
                return value;
            }
        );

auto autoValueToBool = variant_to<KoSvgText::AutoValue> | autoValueToBoolImpl;

// Used for font-size-adjust.
auto autoValueSimplifiedImpl = lager::lenses::getset(
            [](const KoSvgText::AutoValue &value) -> qreal {
                return value.isAuto? 0.0: value.customValue;
            },
            [](KoSvgText::AutoValue value, const qreal &val){
                value.customValue = val;
                value.isAuto = value.customValue > 0? false: true;
                return value;
            }
        );

auto autoValueSimplified = variant_to<KoSvgText::AutoValue> | autoValueSimplifiedImpl;

auto textDecorLinePropImpl = [](KoSvgText::TextDecoration flag) {
    return lager::lenses::getset(
        [flag] (const KoSvgText::TextDecorations &value) -> bool {
            return value.testFlag(flag);
        },
        [flag] (KoSvgText::TextDecorations value, const bool &val){
            value.setFlag(flag, val);
            return value;
        }
    );
};

auto textDecorLineProp = [](KoSvgText::TextDecoration flag) {
    return variant_to<KoSvgText::TextDecorations> | textDecorLinePropImpl(flag);
};

auto textDecorPosPropImpl = [](bool isHorizontal) {
    return lager::lenses::getset(
        [isHorizontal] (const KoSvgText::TextUnderlinePosition &value) -> int {
            return isHorizontal? value.horizontalPosition: value.verticalPosition;
        },
        [isHorizontal] (KoSvgText::TextUnderlinePosition value, const int &val){
            if (isHorizontal) {
                value.horizontalPosition = KoSvgText::TextDecorationUnderlinePosition(val);
            } else {
                value.verticalPosition = KoSvgText::TextDecorationUnderlinePosition(val);
            }
            return value;
        }
    );
};

auto textDecorPosProp = [](bool isHorizontal) {
    return variant_to<KoSvgText::TextUnderlinePosition> | textDecorPosPropImpl(isHorizontal);
};

auto hangPunctuationPropImpl = [](KoSvgText::HangingPunctuation flag) {
    return lager::lenses::getset(
        [flag] (const KoSvgText::HangingPunctuations &value) -> bool {
            return value.testFlag(flag);
        },
        [flag] (KoSvgText::HangingPunctuations value, const bool &val){
            value.setFlag(flag, val);
            return value;
        }
    );
};

auto hangPunctuationProp = [](KoSvgText::HangingPunctuation flag) {
    return variant_to<KoSvgText::HangingPunctuations> | hangPunctuationPropImpl(flag);
};

auto hangingPunactuationCommaPropImpl = lager::lenses::getset(
            [] (const KoSvgText::HangingPunctuations &value) -> KoSvgTextPropertiesModel::HangComma {
                if (value.testFlag(KoSvgText::HangEnd)) {
                    return value.testFlag(KoSvgText::HangForce) ?
                                KoSvgTextPropertiesModel::HangComma::ForceHang :
                                KoSvgTextPropertiesModel::HangComma::AllowHang;
                }
                return KoSvgTextPropertiesModel::NoHang;
            },
            [] (KoSvgText::HangingPunctuations value, const KoSvgTextPropertiesModel::HangComma &val){
                switch (val) {
                case KoSvgTextPropertiesModel::NoHang:
                    value.setFlag(KoSvgText::HangEnd, false);
                    value.setFlag(KoSvgText::HangForce, false);
                    break;
                case KoSvgTextPropertiesModel::AllowHang:
                    value.setFlag(KoSvgText::HangEnd, true);
                    value.setFlag(KoSvgText::HangForce, false);
                    break;
                case KoSvgTextPropertiesModel::ForceHang:
                    value.setFlag(KoSvgText::HangEnd, true);
                    value.setFlag(KoSvgText::HangForce, true);
                    break;
                }
                return value;
            }
        );

auto hangingPunactuationCommaProp =
        variant_to<KoSvgText::HangingPunctuations> | hangingPunactuationCommaPropImpl;

}

auto propertyState = [](const KoSvgTextProperties::PropertyId &propId) { return [propId] (const KoSvgTextPropertyData &value) -> KoSvgTextPropertiesModel::PropertyState {
    if (value.commonProperties.hasProperty(propId)) {
        return KoSvgTextPropertiesModel::PropertySet;
    } else if (value.tristate.contains(propId) || value.inheritedProperties.hasProperty(propId)) {
        return KoSvgTextPropertiesModel::PropertyTriState;
    }
    return KoSvgTextPropertiesModel::PropertyUnset;

};
};

KoSvgTextPropertiesModel::KoSvgTextPropertiesModel(lager::cursor<KoSvgTextPropertyData> _textData)
    : textData(_textData)
    , commonProperties(textData[&KoSvgTextPropertyData::commonProperties])
    , fontSizeData(textData.zoom(createTextProperty(KoSvgTextProperties::FontSizeId)).zoom(variant_to<KoSvgText::CssLengthPercentage>))
    , lineHeightData(textData.zoom(createTextProperty(KoSvgTextProperties::LineHeightId)).zoom(variant_to<KoSvgText::LineHeightInfo>))
    , letterSpacingData(textData.zoom(createTextProperty(KoSvgTextProperties::LetterSpacingId)).zoom(simplifiedAutoLengthProperty))
    , wordSpacingData(textData.zoom(createTextProperty(KoSvgTextProperties::WordSpacingId)).zoom(simplifiedAutoLengthProperty))
    , baselineShiftValueData(textData.zoom(createTextProperty(KoSvgTextProperties::BaselineShiftValueId)).zoom(variant_to<KoSvgText::CssLengthPercentage>))
    , textIndentData(textData.zoom(createTextProperty(KoSvgTextProperties::TextIndentId)).zoom(variant_to<KoSvgText::TextIndentInfo>))
    , tabSizeData(textData.zoom(createTextProperty(KoSvgTextProperties::TabSizeId)).zoom(variant_to<KoSvgText::TabSizeInfo>))
    , textTransformData(textData.zoom(createTextProperty(KoSvgTextProperties::TextTransformId)).zoom(variant_to<KoSvgText::TextTransformInfo>))
    , cssFontStyleData(textData.zoom(createTextProperty(KoSvgTextProperties::FontStyleId)).zoom(variant_to<KoSvgText::CssFontStyleData>))
    , fontVariantLigaturesData(textData.zoom(createTextProperty(KoSvgTextProperties::FontVariantLigatureId)).zoom(variant_to<KoSvgText::FontFeatureLigatures>))
    , fontVariantNumericData(textData.zoom(createTextProperty(KoSvgTextProperties::FontVariantNumericId)).zoom(variant_to<KoSvgText::FontFeatureNumeric>))
    , fontVariantEastAsianData(textData.zoom(createTextProperty(KoSvgTextProperties::FontVariantEastAsianId)).zoom(variant_to<KoSvgText::FontFeatureEastAsian>))
    , fontSizeModel(fontSizeData)
    , lineHeightModel(lineHeightData)
    , letterSpacingModel(letterSpacingData)
    , wordSpacingModel(wordSpacingData)
    , baselineShiftValueModel(baselineShiftValueData)
    , textIndentModel(textIndentData)
    , tabSizeModel(tabSizeData)
    , textTransformModel(textTransformData)
    , cssFontStyleModel(cssFontStyleData)
    , fontVariantLigaturesModel(fontVariantLigaturesData)
    , fontVariantNumericModel(fontVariantNumericData)
    , fontVariantEastAsianModel(fontVariantEastAsianData)
    , LAGER_QT(fontSizeState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontSizeId))}
    , LAGER_QT(lineHeightState) {textData.zoom(propertyModifyState(KoSvgTextProperties::LineHeightId))}
    , LAGER_QT(letterSpacingState) {textData.zoom(propertyModifyState(KoSvgTextProperties::LetterSpacingId))}
    , LAGER_QT(wordSpacingState) {textData.zoom(propertyModifyState(KoSvgTextProperties::WordSpacingId))}
    , LAGER_QT(textIndentState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextIndentId))}
    , LAGER_QT(tabSizeState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TabSizeId))}
    , LAGER_QT(textTransformState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextTransformId))}
    , LAGER_QT(fontStyleState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontStyleId))}
    , LAGER_QT(writingMode) {textData.zoom(createTextProperty(KoSvgTextProperties::WritingModeId)).zoom(variant_to<int>)}
    , LAGER_QT(writingModeState) {textData.zoom(propertyModifyState(KoSvgTextProperties::WritingModeId))}
    , LAGER_QT(direction) {textData.zoom(createTextProperty(KoSvgTextProperties::DirectionId)).zoom(variant_to<int>)}
    , LAGER_QT(directionState) {textData.zoom(propertyModifyState(KoSvgTextProperties::DirectionId))}
    , LAGER_QT(unicodeBidi) {textData.zoom(createTextProperty(KoSvgTextProperties::UnicodeBidiId)).zoom(variant_to<int>)}
    , LAGER_QT(unicodeBidiState) {textData.zoom(propertyModifyState(KoSvgTextProperties::UnicodeBidiId))}
    , LAGER_QT(textAlignAll) {textData.zoom(createTextProperty(KoSvgTextProperties::TextAlignAllId)).zoom(variant_to<int>)}
    , LAGER_QT(textAlignAllState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextAlignAllId))}
    , LAGER_QT(textAlignLast) {textData.zoom(createTextProperty(KoSvgTextProperties::TextAlignLastId)).zoom(variant_to<int>)}
    , LAGER_QT(textAlignLastState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextAlignLastId))}
    , LAGER_QT(textAnchor) {textData.zoom(createTextProperty(KoSvgTextProperties::TextAnchorId)).zoom(variant_to<int>)}
    , LAGER_QT(textAnchorState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextAnchorId))}
    , LAGER_QT(fontWeight) {textData.zoom(createTextProperty(KoSvgTextProperties::FontWeightId)).zoom(variant_to<int>)}
    , LAGER_QT(fontWeightState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontWeightId))}
    , LAGER_QT(fontWidth) {textData.zoom(createTextProperty(KoSvgTextProperties::FontStretchId)).zoom(variant_to<int>)}
    , LAGER_QT(fontWidthState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontStretchId))}
    , LAGER_QT(fontOpticalSizeLink) {textData.zoom(createTextProperty(KoSvgTextProperties::FontOpticalSizingId)).zoom(variant_to<bool>)}
    , LAGER_QT(fontOpticalSizeLinkState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontOpticalSizingId))}
    , LAGER_QT(fontFamilies) {textData.zoom(createTextProperty(KoSvgTextProperties::FontFamiliesId)).zoom(variant_to<QStringList>)}
    , LAGER_QT(fontFamiliesState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontFamiliesId))}
    , LAGER_QT(axisValues) {textData.zoom(createTextProperty(KoSvgTextProperties::FontVariationSettingsId)).zoom(variant_to<QVariantMap>)}
    , LAGER_QT(axisValuesState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontVariationSettingsId))}
    , LAGER_QT(textDecorationUnderline){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationUnderline))}
    , LAGER_QT(textDecorationOverline){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationOverline))}
    , LAGER_QT(textDecorationLineThrough){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationLineThrough))}
    , LAGER_QT(textDecorationLineState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextDecorationLineId))}
    , LAGER_QT(textDecorationStyle){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationStyleId)).zoom(variant_to<int>)}
    , LAGER_QT(textDecorationStyleState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextDecorationStyleId))}
    , LAGER_QT(textDecorationColor){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationColorId)).zoom(variant_to<QColor>)}
    , LAGER_QT(textDecorationColorState){textData.zoom(propertyModifyState(KoSvgTextProperties::TextDecorationColorId))}
    , LAGER_QT(textDecorationUnderlinePosHorizontal){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationPositionId)).zoom(textDecorPosProp(true))}
    , LAGER_QT(textDecorationUnderlinePosVertical){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationPositionId)).zoom(textDecorPosProp(false))}
    , LAGER_QT(textDecorationUnderlinePositionState){textData.zoom(propertyModifyState(KoSvgTextProperties::TextDecorationPositionId))}
    , LAGER_QT(hangingPunctuationFirst){textData.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangPunctuationProp(KoSvgText::HangFirst))}
    , LAGER_QT(hangingPunctuationComma){textData.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangingPunactuationCommaProp)}
    , LAGER_QT(hangingPunctuationLast){textData.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangPunctuationProp(KoSvgText::HangLast))}
    , LAGER_QT(hangingPunctuationState) {textData.zoom(propertyModifyState(KoSvgTextProperties::HangingPunctuationId))}
    , LAGER_QT(alignmentBaseline){textData.zoom(createTextProperty(KoSvgTextProperties::AlignmentBaselineId)).zoom(variant_to<int>)}
    , LAGER_QT(alignmentBaselineState) {textData.zoom(propertyModifyState(KoSvgTextProperties::AlignmentBaselineId))}
    , LAGER_QT(dominantBaseline){textData.zoom(createTextProperty(KoSvgTextProperties::DominantBaselineId)).zoom(variant_to<int>)}
    , LAGER_QT(dominantBaselineState) {textData.zoom(propertyModifyState(KoSvgTextProperties::DominantBaselineId))}
    , LAGER_QT(baselineShiftMode){textData.zoom(createTextProperty(KoSvgTextProperties::BaselineShiftModeId)).zoom(variant_to<int>)}
    , LAGER_QT(baselineShiftState) {textData.zoom(propertyModifyState(KoSvgTextProperties::BaselineShiftModeId))}
    , LAGER_QT(wordBreak){textData.zoom(createTextProperty(KoSvgTextProperties::WordBreakId)).zoom(variant_to<int>)}
    , LAGER_QT(wordBreakState) {textData.zoom(propertyModifyState(KoSvgTextProperties::WordBreakId))}
    , LAGER_QT(lineBreak){textData.zoom(createTextProperty(KoSvgTextProperties::LineBreakId)).zoom(variant_to<int>)}
    , LAGER_QT(lineBreakState) {textData.zoom(propertyModifyState(KoSvgTextProperties::LineBreakId))}
    , LAGER_QT(fontSynthesisWeight){textData.zoom(createTextProperty(KoSvgTextProperties::FontSynthesisBoldId)).zoom(variant_to<bool>)}
    , LAGER_QT(fontSynthesisWeightState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontSynthesisBoldId))}
    , LAGER_QT(fontSynthesisStyle){textData.zoom(createTextProperty(KoSvgTextProperties::FontSynthesisItalicId)).zoom(variant_to<bool>)}
    , LAGER_QT(fontSynthesisStyleState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontSynthesisItalicId))}
    , LAGER_QT(fontVariantPosition){textData.zoom(createTextProperty(KoSvgTextProperties::FontVariantPositionId)).zoom(variant_to<int>)}
    , LAGER_QT(fontVariantPositionState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontVariantPositionId))}
    , LAGER_QT(fontVariantCaps){textData.zoom(createTextProperty(KoSvgTextProperties::FontVariantCapsId)).zoom(variant_to<int>)}
    , LAGER_QT(fontVariantCapsState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontVariantCapsId))}
    , LAGER_QT(fontVariantLigaturesState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontVariantLigatureId))}
    , LAGER_QT(fontVariantNumericState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontVariantNumericId))}
    , LAGER_QT(fontVariantEastAsianState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontVariantEastAsianId))}
    , LAGER_QT(fontFeatureSettings) {textData.zoom(createTextProperty(KoSvgTextProperties::FontFeatureSettingsId)).zoom(variant_to<QVariantMap>)}
    , LAGER_QT(fontFeatureSettingsState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontFeatureSettingsId))}
    , LAGER_QT(fontKerning) {textData.zoom(createTextProperty(KoSvgTextProperties::KerningId)).zoom(autoValueToBool)}
    , LAGER_QT(fontKerningState) {textData.zoom(propertyModifyState(KoSvgTextProperties::KerningId))}
    , LAGER_QT(language) {textData.zoom(createTextProperty(KoSvgTextProperties::TextLanguage)).zoom(variant_to<QString>)}
    , LAGER_QT(languageState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextLanguage))}
    , LAGER_QT(fontSizeAdjust) {textData.zoom(createTextProperty(KoSvgTextProperties::FontSizeAdjustId)).zoom(autoValueSimplified)}
    , LAGER_QT(fontSizeAdjustState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontSizeAdjustId))}
    , LAGER_QT(textCollapse){textData.zoom(createTextProperty(KoSvgTextProperties::TextCollapseId)).zoom(variant_to<int>)}
    , LAGER_QT(textCollapseState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextCollapseId))}
    , LAGER_QT(textWrap){textData.zoom(createTextProperty(KoSvgTextProperties::TextWrapId)).zoom(variant_to<int>)}
    , LAGER_QT(textWrapState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextWrapId))}
    , LAGER_QT(textRendering){textData.zoom(createTextProperty(KoSvgTextProperties::TextRenderingId)).zoom(variant_to<int>)}
    , LAGER_QT(textRenderingState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextRenderingId))}
    , LAGER_QT(spanSelection) {textData[&KoSvgTextPropertyData::spanSelection]}
{
    lager::watch(textData, std::bind(&KoSvgTextPropertiesModel::textPropertyChanged, this));
    lager::watch(fontSizeData, std::bind(&KoSvgTextPropertiesModel::fontSizeChanged, this));
    lager::watch(lineHeightData, std::bind(&KoSvgTextPropertiesModel::lineHeightChanged, this));

    lager::watch(letterSpacingData, std::bind(&KoSvgTextPropertiesModel::letterSpacingChanged, this));
    lager::watch(wordSpacingData, std::bind(&KoSvgTextPropertiesModel::wordSpacingChanged, this));
    lager::watch(baselineShiftValueData, std::bind(&KoSvgTextPropertiesModel::baselineShiftValueChanged, this));

    lager::watch(textIndentData, std::bind(&KoSvgTextPropertiesModel::textIndentChanged, this));
    connect(&textIndentModel, SIGNAL(lengthChanged()), this, SIGNAL(textIndentChanged()));
    lager::watch(tabSizeData, std::bind(&KoSvgTextPropertiesModel::tabSizeChanged, this));
    lager::watch(textTransformData, std::bind(&KoSvgTextPropertiesModel::textTransformChanged, this));
    lager::watch(cssFontStyleData, std::bind(&KoSvgTextPropertiesModel::fontStyleChanged, this));

    lager::watch(fontVariantLigaturesData, std::bind(&KoSvgTextPropertiesModel::fontVariantLigaturesChanged, this));
    lager::watch(fontVariantNumericData, std::bind(&KoSvgTextPropertiesModel::fontVariantNumericChanged, this));
    lager::watch(fontVariantEastAsianData, std::bind(&KoSvgTextPropertiesModel::fontVariantEastAsianChanged, this));
}

CssLengthPercentageModel *KoSvgTextPropertiesModel::fontSize()
{
    return &this->fontSizeModel;
}

LineHeightModel *KoSvgTextPropertiesModel::lineHeight()
{
    return &this->lineHeightModel;
}

CssLengthPercentageModel *KoSvgTextPropertiesModel::letterSpacing()
{
    return &this->letterSpacingModel;
}

CssLengthPercentageModel *KoSvgTextPropertiesModel::wordSpacing()
{
    return &this->wordSpacingModel;
}

CssLengthPercentageModel *KoSvgTextPropertiesModel::baselineShiftValue()
{
    return &this->baselineShiftValueModel;
}

TextIndentModel *KoSvgTextPropertiesModel::textIndent()
{
    return &this->textIndentModel;
}

TabSizeModel *KoSvgTextPropertiesModel::tabSize()
{
    return &this->tabSizeModel;
}

TextTransformModel *KoSvgTextPropertiesModel::textTransform()
{
    return &this->textTransformModel;
}

CssFontStyleModel *KoSvgTextPropertiesModel::fontStyle()
{
    return &this->cssFontStyleModel;
}

FontVariantLigaturesModel *KoSvgTextPropertiesModel::fontVariantLigatures()
{
    return &this->fontVariantLigaturesModel;
}

FontVariantNumericModel *KoSvgTextPropertiesModel::fontVariantNumeric()
{
    return &this->fontVariantNumericModel;
}

FontVariantEastAsianModel *KoSvgTextPropertiesModel::fontVariantEastAsian()
{
    return &this->fontVariantEastAsianModel;
}

qreal KoSvgTextPropertiesModel::resolvedFontSize(bool fontSize)
{
    KoSvgTextPropertyData data = textData.get();
    KoSvgTextProperties inherited = data.inheritedProperties;
    inherited.inheritFrom(KoSvgTextProperties::defaultProperties(), true);
    if (fontSize) {
        return inherited.fontSize().value;
    } else {
        KoSvgTextProperties commonProperties = data.commonProperties;
        commonProperties.inheritFrom(inherited, true);
        return commonProperties.fontSize().value;
    }
}

qreal KoSvgTextPropertiesModel::resolvedXHeight(bool fontSize)
{
    KoSvgTextPropertyData data = textData.get();
    KoSvgTextProperties inherited = data.inheritedProperties;
    inherited.inheritFrom(KoSvgTextProperties::defaultProperties(), true);
    if (fontSize) {
        return inherited.xHeight();
    } else {
        KoSvgTextProperties commonProperties = data.commonProperties;
        commonProperties.inheritFrom(inherited, true);
        return commonProperties.xHeight();
    }
}
