/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextPropertiesModel.h"
#include "KoSvgTextProperties.h"
#include <lager/constant.hpp>
#include <QDebug>

namespace {

auto createCommonProperties = lager::lenses::getset (
            [] (const KoSvgTextPropertyData &value) -> KoSvgTextProperties {
    return value.commonProperties;
},
[] (KoSvgTextPropertyData data, const KoSvgTextProperties &props) -> KoSvgTextPropertyData {
    data.commonProperties = props;
    return data;
}
            );

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
        } else if (value.tristate.contains(propId) || value.inheritedProperties.hasProperty(propId)) {
            return KoSvgTextPropertiesModel::PropertyTriState;
        }
        return KoSvgTextPropertiesModel::PropertyUnset;
    },
    [propId](KoSvgTextPropertyData value, const KoSvgTextPropertiesModel::PropertyState &state) -> KoSvgTextPropertyData {
        if (state == KoSvgTextPropertiesModel::PropertySet) {
            value.commonProperties.setProperty(propId, value.inheritedProperties.propertyOrDefault(propId));
        } else {
            // Because tristate represents properties that are set but mixed, there's no value in being able to set them from the UX.
            // so if we receive such a signal, it is incorrect.
            if (state == KoSvgTextPropertiesModel::PropertyTriState) {
                qWarning() << "Receiving request to set property tristate, this will unset the property instead";
            }
            value.commonProperties.removeProperty(propId);
        }
        value.tristate.remove(propId);
        return value;
    }
    );
};

auto integerProperty = lager::lenses::getset(
            [](const QVariant &value) -> int {return value.toInt();},
            [](QVariant value, const int &val){value = QVariant::fromValue(val); return value;});
auto boolProperty = lager::lenses::getset(
            [](const QVariant &value) -> bool {return value.toBool();},
            [](QVariant value, const bool &val){value = QVariant::fromValue(val); return value;});
auto lengthPercentageProperty = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgText::CssLengthPercentage {return value.value<KoSvgText::CssLengthPercentage>();},
            [](QVariant value, const KoSvgText::CssLengthPercentage &val){value = QVariant::fromValue(val); return value;});
auto simplifiedAutoLengthProperty = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgText::CssLengthPercentage {
                KoSvgText::AutoLengthPercentage length = value.value<KoSvgText::AutoLengthPercentage>();
                return length.isAuto? KoSvgText::CssLengthPercentage(): length.length;
            },
            [](QVariant value, const KoSvgText::CssLengthPercentage &val){
                value = QVariant::fromValue(KoSvgText::AutoLengthPercentage(val)); return value;
            }
        );
auto stringListProperty = lager::lenses::getset(
            [](const QVariant &value) -> QStringList {return value.value<QStringList>();},
            [](QVariant value, const QStringList &val){value = QVariant::fromValue(val); return value;});
auto lineHeightProperty = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgText::LineHeightInfo {return value.value<KoSvgText::LineHeightInfo>();},
            [](QVariant value, const KoSvgText::LineHeightInfo &val){value = QVariant::fromValue(val); return value;});
auto textIndentProperty = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgText::TextIndentInfo {return value.value<KoSvgText::TextIndentInfo>();},
            [](QVariant value, const KoSvgText::TextIndentInfo &val){value = QVariant::fromValue(val); return value;});
auto tabSizeProperty = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgText::TabSizeInfo {return value.value<KoSvgText::TabSizeInfo>();},
            [](QVariant value, const KoSvgText::TabSizeInfo &val){value = QVariant::fromValue(val); return value;});
auto textTransformProperty = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgText::TextTransformInfo {return value.value<KoSvgText::TextTransformInfo>();},
            [](QVariant value, const KoSvgText::TextTransformInfo &val){value = QVariant::fromValue(val); return value;});
auto textDecorLineProp = [](KoSvgText::TextDecoration flag) {
    return lager::lenses::getset(
        [flag](const QVariant &value) -> bool {
            return value.value<KoSvgText::TextDecorations>().testFlag(flag);
        },
        [flag](QVariant value, const bool &val){
            KoSvgText::TextDecorations decor = value.value<KoSvgText::TextDecorations>();
            decor.setFlag(flag, val);
            value = QVariant::fromValue(decor);
            return value;
        }
    );
};
auto hangPunctuationProp = [](KoSvgText::HangingPunctuation flag) {
    return lager::lenses::getset(
        [flag](const QVariant &value) -> bool {
            return value.value<KoSvgText::HangingPunctuations>().testFlag(flag);
        },
        [flag](QVariant value, const bool &val){
            KoSvgText::HangingPunctuations hang = value.value<KoSvgText::HangingPunctuations>();
            hang.setFlag(flag, val);
            value = QVariant::fromValue(hang);
            return value;
        }
    );
};
auto hangingPunactuationCommaProp = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgTextPropertiesModel::HangComma {
                KoSvgText::HangingPunctuations hang = value.value<KoSvgText::HangingPunctuations>();
                if (hang.testFlag(KoSvgText::HangEnd)) {
                    return hang.testFlag(KoSvgText::HangForce)?
                                KoSvgTextPropertiesModel::HangComma::ForceHang:
                                KoSvgTextPropertiesModel::HangComma::AllowHang;
                }
                return KoSvgTextPropertiesModel::NoHang;
            },
            [](QVariant value, const KoSvgTextPropertiesModel::HangComma &val){
            KoSvgText::HangingPunctuations hang = value.value<KoSvgText::HangingPunctuations>();
            switch (val) {
            case KoSvgTextPropertiesModel::NoHang:
                hang.setFlag(KoSvgText::HangEnd, false);
                hang.setFlag(KoSvgText::HangForce, false);
                break;
            case KoSvgTextPropertiesModel::AllowHang:
                hang.setFlag(KoSvgText::HangEnd, true);
                hang.setFlag(KoSvgText::HangForce, false);
                break;
            case KoSvgTextPropertiesModel::ForceHang:
                hang.setFlag(KoSvgText::HangEnd, true);
                hang.setFlag(KoSvgText::HangForce, true);
                break;
            }
            value = QVariant::fromValue(hang);
            return value;
            }
        );
auto fontStyleProperty = lager::lenses::getset(
            [](const QVariant &value) -> KoSvgTextPropertiesModel::FontStyle {return KoSvgTextPropertiesModel::FontStyle(value.toInt());},
            [](QVariant value, const KoSvgTextPropertiesModel::FontStyle &val){value = QVariant::fromValue(val); return value;});
auto qColorProperty = lager::lenses::getset(
            [](const QVariant &value) -> QColor {return value.value<QColor>();},
            [](QVariant value, const QColor &val){value = QVariant::fromValue(val); return value;});


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
    , commonProperties(textData.zoom(createCommonProperties))
    , fontSizeData(textData.zoom(createTextProperty(KoSvgTextProperties::FontSizeId)).zoom(lengthPercentageProperty))
    , lineHeightData(textData.zoom(createTextProperty(KoSvgTextProperties::LineHeightId)).zoom(lineHeightProperty))
    , letterSpacingData(textData.zoom(createTextProperty(KoSvgTextProperties::LetterSpacingId)).zoom(simplifiedAutoLengthProperty))
    , wordSpacingData(textData.zoom(createTextProperty(KoSvgTextProperties::WordSpacingId)).zoom(simplifiedAutoLengthProperty))
    , baselineShiftValueData(textData.zoom(createTextProperty(KoSvgTextProperties::BaselineShiftValueId)).zoom(lengthPercentageProperty))
    , textIndentData(textData.zoom(createTextProperty(KoSvgTextProperties::TextIndentId)).zoom(textIndentProperty))
    , tabSizeData(textData.zoom(createTextProperty(KoSvgTextProperties::TabSizeId)).zoom(tabSizeProperty))
    , textTransformData(textData.zoom(createTextProperty(KoSvgTextProperties::TextTransformId)).zoom(textTransformProperty))
    , fontSizeModel(fontSizeData)
    , lineHeightModel(lineHeightData)
    , letterSpacingModel(letterSpacingData)
    , wordSpacingModel(wordSpacingData)
    , baselineShiftValueModel(baselineShiftValueData)
    , textIndentModel(textIndentData)
    , tabSizeModel(tabSizeData)
    , textTransformModel(textTransformData)
    , LAGER_QT(fontSizeState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontSizeId))}
    , LAGER_QT(lineHeightState) {textData.zoom(propertyModifyState(KoSvgTextProperties::LineHeightId))}
    , LAGER_QT(letterSpacingState) {textData.zoom(propertyModifyState(KoSvgTextProperties::LetterSpacingId))}
    , LAGER_QT(wordSpacingState) {textData.zoom(propertyModifyState(KoSvgTextProperties::WordSpacingId))}
    , LAGER_QT(textIndentState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextIndentId))}
    , LAGER_QT(tabSizeState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TabSizeId))}
    , LAGER_QT(textTransformState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextTransformId))}
    , LAGER_QT(writingMode) {textData.zoom(createTextProperty(KoSvgTextProperties::WritingModeId)).zoom(integerProperty)}
    , LAGER_QT(writingModeState) {textData.zoom(propertyModifyState(KoSvgTextProperties::WritingModeId))}
    , LAGER_QT(direction) {textData.zoom(createTextProperty(KoSvgTextProperties::DirectionId)).zoom(integerProperty)}
    , LAGER_QT(directionState) {textData.zoom(propertyModifyState(KoSvgTextProperties::DirectionId))}
    , LAGER_QT(textAlignAll) {textData.zoom(createTextProperty(KoSvgTextProperties::TextAlignAllId)).zoom(integerProperty)}
    , LAGER_QT(textAlignAllState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextAlignAllId))}
    , LAGER_QT(textAlignLast) {textData.zoom(createTextProperty(KoSvgTextProperties::TextAlignLastId)).zoom(integerProperty)}
    , LAGER_QT(textAlignLastState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextAlignLastId))}
    , LAGER_QT(textAnchor) {textData.zoom(createTextProperty(KoSvgTextProperties::TextAnchorId)).zoom(integerProperty)}
    , LAGER_QT(textAnchorState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextAnchorId))}
    , LAGER_QT(fontWeight) {textData.zoom(createTextProperty(KoSvgTextProperties::FontWeightId)).zoom(integerProperty)}
    , LAGER_QT(fontWeightState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontWeightId))}
    , LAGER_QT(fontWidth) {textData.zoom(createTextProperty(KoSvgTextProperties::FontStretchId)).zoom(integerProperty)}
    , LAGER_QT(fontWidthState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontStretchId))}
    , LAGER_QT(fontStyle) {textData.zoom(createTextProperty(KoSvgTextProperties::FontStyleId)).zoom(fontStyleProperty)}
    , LAGER_QT(fontStyleState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontStyleId))}
    , LAGER_QT(fontOpticalSizeLink) {textData.zoom(createTextProperty(KoSvgTextProperties::FontOpticalSizingId)).zoom(boolProperty)}
    , LAGER_QT(fontOpticalSizeLinkState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontOpticalSizingId))}
    , LAGER_QT(fontFamilies) {textData.zoom(createTextProperty(KoSvgTextProperties::FontFamiliesId)).zoom(stringListProperty)}
    , LAGER_QT(fontFamiliesState) {textData.zoom(propertyModifyState(KoSvgTextProperties::FontFamiliesId))}
    , LAGER_QT(textDecorationUnderline){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationUnderline))}
    , LAGER_QT(textDecorationOverline){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationOverline))}
    , LAGER_QT(textDecorationLineThrough){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationLineThrough))}
    , LAGER_QT(textDecorationLineState) {textData.zoom(propertyModifyState(KoSvgTextProperties::TextDecorationLineId))}
    , LAGER_QT(textDecorationStyle){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationStyleId)).zoom(integerProperty)}
    , LAGER_QT(textDecorationColor){textData.zoom(createTextProperty(KoSvgTextProperties::TextDecorationColorId)).zoom(qColorProperty)}
    , LAGER_QT(hangingPunctuationFirst){textData.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangPunctuationProp(KoSvgText::HangFirst))}
    , LAGER_QT(hangingPunctuationComma){textData.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangingPunactuationCommaProp)}
    , LAGER_QT(hangingPunctuationLast){textData.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangPunctuationProp(KoSvgText::HangLast))}
    , LAGER_QT(hangingPunctuationState) {textData.zoom(propertyModifyState(KoSvgTextProperties::HangingPunctuationId))}
    , LAGER_QT(alignmentBaseline){textData.zoom(createTextProperty(KoSvgTextProperties::AlignmentBaselineId)).zoom(integerProperty)}
    , LAGER_QT(alignmentBaselineState) {textData.zoom(propertyModifyState(KoSvgTextProperties::AlignmentBaselineId))}
    , LAGER_QT(dominantBaseline){textData.zoom(createTextProperty(KoSvgTextProperties::DominantBaselineId)).zoom(integerProperty)}
    , LAGER_QT(dominantBaselineState) {textData.zoom(propertyModifyState(KoSvgTextProperties::DominantBaselineId))}
    , LAGER_QT(baselineShiftMode){textData.zoom(createTextProperty(KoSvgTextProperties::BaselineShiftModeId)).zoom(integerProperty)}
    , LAGER_QT(baselineShiftState) {textData.zoom(propertyModifyState(KoSvgTextProperties::BaselineShiftModeId))}
    , LAGER_QT(wordBreak){textData.zoom(createTextProperty(KoSvgTextProperties::WordBreakId)).zoom(integerProperty)}
    , LAGER_QT(wordBreakState) {textData.zoom(propertyModifyState(KoSvgTextProperties::WordBreakId))}
    , LAGER_QT(lineBreak){textData.zoom(createTextProperty(KoSvgTextProperties::LineBreakId)).zoom(integerProperty)}
    , LAGER_QT(lineBreakState) {textData.zoom(propertyModifyState(KoSvgTextProperties::LineBreakId))}
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
