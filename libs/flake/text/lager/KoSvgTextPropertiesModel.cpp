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
                [propId](const KoSvgTextProperties &value) -> QVariant {
        return value.propertyOrDefault(propId);
    },
    [propId](KoSvgTextProperties value, const QVariant &variant) -> KoSvgTextProperties {
        value.setProperty(propId, variant);
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

KoSvgTextPropertiesModel::KoSvgTextPropertiesModel(lager::cursor<KoSvgTextPropertyData> _textData)
    : textData(_textData)
    , commonProperties(textData.zoom(createCommonProperties))
    , fontSizeData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::FontSizeId)).zoom(lengthPercentageProperty))
    , lineHeightData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::LineHeightId)).zoom(lineHeightProperty))
    , letterSpacingData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::LetterSpacingId)).zoom(simplifiedAutoLengthProperty))
    , wordSpacingData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::WordSpacingId)).zoom(simplifiedAutoLengthProperty))
    , baselineShiftValueData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::BaselineShiftValueId)).zoom(lengthPercentageProperty))
    , textIndentData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextIndentId)).zoom(textIndentProperty))
    , tabSizeData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::TabSizeId)).zoom(tabSizeProperty))
    , textTransformData(commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextTransformId)).zoom(textTransformProperty))
    , fontSizeModel(fontSizeData)
    , lineHeightModel(lineHeightData)
    , letterSpacingModel(letterSpacingData)
    , wordSpacingModel(wordSpacingData)
    , baselineShiftValueModel(baselineShiftValueData)
    , textIndentModel(textIndentData)
    , tabSizeModel(tabSizeData)
    , textTransformModel(textTransformData)
    , LAGER_QT(writingMode) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::WritingModeId)).zoom(integerProperty)}
    , LAGER_QT(direction) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::DirectionId)).zoom(integerProperty)}
    , LAGER_QT(textAlignAll) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextAlignAllId)).zoom(integerProperty)}
    , LAGER_QT(textAlignLast) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextAlignLastId)).zoom(integerProperty)}
    , LAGER_QT(textAnchor) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextAnchorId)).zoom(integerProperty)}
    , LAGER_QT(fontWeight) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::FontWeightId)).zoom(integerProperty)}
    , LAGER_QT(fontWidth) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::FontStretchId)).zoom(integerProperty)}
    , LAGER_QT(fontStyle) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::FontStyleId)).zoom(fontStyleProperty)}
    , LAGER_QT(fontOpticalSizeLink) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::FontOpticalSizingId)).zoom(boolProperty)}
    , LAGER_QT(fontFamilies) {commonProperties.zoom(createTextProperty(KoSvgTextProperties::FontFamiliesId)).zoom(stringListProperty)}
    , LAGER_QT(textDecorationUnderline){commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationUnderline))}
    , LAGER_QT(textDecorationOverline){commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationOverline))}
    , LAGER_QT(textDecorationLineThrough){commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextDecorationLineId)).zoom(textDecorLineProp(KoSvgText::DecorationLineThrough))}
    , LAGER_QT(textDecorationStyle){commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextDecorationStyleId)).zoom(integerProperty)}
    , LAGER_QT(textDecorationColor){commonProperties.zoom(createTextProperty(KoSvgTextProperties::TextDecorationColorId)).zoom(qColorProperty)}
    , LAGER_QT(hangingPunctuationFirst){commonProperties.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangPunctuationProp(KoSvgText::HangFirst))}
    , LAGER_QT(hangingPunctuationComma){commonProperties.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangingPunactuationCommaProp)}
    , LAGER_QT(hangingPunctuationLast){commonProperties.zoom(createTextProperty(KoSvgTextProperties::HangingPunctuationId)).zoom(hangPunctuationProp(KoSvgText::HangLast))}
    , LAGER_QT(alignmentBaseline){commonProperties.zoom(createTextProperty(KoSvgTextProperties::AlignmentBaselineId)).zoom(integerProperty)}
    , LAGER_QT(dominantBaseline){commonProperties.zoom(createTextProperty(KoSvgTextProperties::DominantBaselineId)).zoom(integerProperty)}
    , LAGER_QT(baselineShiftMode){commonProperties.zoom(createTextProperty(KoSvgTextProperties::BaselineShiftModeId)).zoom(integerProperty)}
    , LAGER_QT(wordBreak){commonProperties.zoom(createTextProperty(KoSvgTextProperties::WordBreakId)).zoom(integerProperty)}
    , LAGER_QT(lineBreak){commonProperties.zoom(createTextProperty(KoSvgTextProperties::LineBreakId)).zoom(integerProperty)}
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
