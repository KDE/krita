/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_hdr_metadata_models.h"
#include <KLocalizedString>
#include <lager/constant.hpp>
#include <lager/setter.hpp>
#include <KisZug.h>
#include <KisLager.h>
namespace {

static constexpr std::array<double, 2> refWhiteList {
    80.0,
    203.0
};

double calcEffectiveReferenceWhite(bool imageProfileIsRelative, double imageProfileReferenceWhite, double imageMetadataReferenceWhite)
{
    return imageProfileIsRelative ? imageMetadataReferenceWhite : imageProfileReferenceWhite;
}

auto referenceWhiteToInt =
    lager::lenses::getset(
        [] (const double &value) -> int {
            int index = -1;

            for (size_t i = 0; i < refWhiteList.size(); i++) {
                if (qFuzzyCompare(value, refWhiteList.at(i))) {
                    index = i;
                    break;
                }
            }
            return index;
        },
        [] (double value, const int &newValue) {
            if (newValue >= 0 && newValue < int(refWhiteList.size())) {
                value = refWhiteList[newValue];
            }
            return value;
        }
        );

ComboBoxState calcWhiteTypeToComboBoxState(int index, bool enabled)
{
    QStringList values;
    QStringList toolTips;

    for (size_t i = 0; i < refWhiteList.size(); i++) {
        values << i18nc("Image reference white value", "%1 cd/m²", QString::number(refWhiteList.at(i)));
        toolTips << "";
    }

    return {values, index, enabled, toolTips};
}

ComboBoxState calcTypeComboBoxState(KisRelativeContentLightLevelInformation::CalculationType type, bool enabled)
{
    QStringList values;
    QStringList toolTips;

    values << i18nc("CLLI calculation type", "XYZ Luminance");
    values << i18nc("CLLI calculation type", "Rec 2020 Per Component");
    values << i18nc("CLLI calculation type", "RGB Per Component");

    toolTips << i18nc("@info:tooltip", "Calculate the brightness in nits against XYZ luminance.");
    toolTips << i18nc("@info:tooltip", "Calculate the brightness in nits by testing the components in linear rec 2020.");
    toolTips << i18nc("@info:tooltip", "Calculate the brightness in nits by testing the components in linear RGB, if possible, falls back to using XYZ luminance.");

    return {values, static_cast<int>(type), enabled, toolTips};
}

auto noopSetter = [] (auto) {};

auto calcRelativeToAbsoluteWhiteLevel = lager::lenses::getset(
        // .first is relative white level
        // .second is reference white level
        [] (const std::tuple<double,double> &packed) -> double {
            return std::get<0>(packed) * std::get<1>(packed);
        },
        [] (std::tuple<double,double> packed, double absoluteValue) {
            if (!qFuzzyIsNull(std::get<1>(packed))) {
                std::get<0>(packed) = absoluteValue / std::get<1>(packed);
            }
            return packed;
        }
);

auto effectivePresetIndexForCvi =
    lager::lenses::getset(
        [] (const KisColorVolumeInformation &cvi) -> int {
            if (cvi == KisColorVolumeInformation::BT2100PQ) {
                return 0;
            } else if (cvi == KisColorVolumeInformation::DCIP3D65) {
                return 1;
            } else {
                return 2;
            }
        },
        [] (KisColorVolumeInformation cvi, const int &index) {
            if (index == 0) {
                cvi = KisColorVolumeInformation::BT2100PQ;
            } else if (index == 1) {
                cvi = KisColorVolumeInformation::DCIP3D65;
            } else {
                // noop, just keep `cvi` unchanged
            }
            return cvi;
        }
    );

 ComboBoxState calcCviEffectivePresetIndexState(int index, bool enabled) {
    QStringList values;
    QStringList toolTips;

    values << i18n("Rec. 2100 PQ");
    values << i18n("DCI-P3 D65");
    values << i18n("Custom");

    toolTips << i18nc("@info:tooltip", "Reset color volume to the values defined by Rec. 2100 PQ");
    toolTips << i18nc("@info:tooltip", "Reset color volume to the values defined by DCI-P3 D65");
    toolTips << i18nc("@info:tooltip", "Use custom values for color volume");

    return {values, index, enabled, toolTips};
}

}

using KisWidgetConnectionUtils::ToControlState;

KisHDRMetadataModel::KisHDRMetadataModel(QObject *parent)
    : QObject{parent}
    , clliData(lager::make_state(KisRelativeContentLightLevelInformation(), lager::automatic_tag{}))
    , cviData(lager::make_state(KisColorVolumeInformation(), lager::automatic_tag{}))
    , imageProfileReferenceWhiteData(lager::make_state(203.0, lager::automatic_tag{}))
    , referenceWhiteData(lager::make_state(203.0, lager::automatic_tag{}))
    , imageProfileIsRelativeData(lager::make_state(false, lager::automatic_tag{}))
    , referenceWhitePresentData(lager::make_state(false, lager::automatic_tag{}))
    , clliPresentData(lager::make_state(false, lager::automatic_tag{}))
    , cviPresentData(lager::make_state(false, lager::automatic_tag{}))

    , LAGER_QT(referenceWhitePresent) {referenceWhitePresentData}
    , LAGER_QT(effectiveReferenceWhitePresent){
        lager::with(referenceWhitePresentData,
                    imageProfileIsRelativeData.map(std::logical_not<>{})
        ).map(std::logical_or<>{})}
    , LAGER_QT(effectiveReferenceWhiteEnabled){imageProfileIsRelativeData}
    , LAGER_QT(referenceWhitePresentState){lager::with(LAGER_QT(effectiveReferenceWhitePresent), LAGER_QT(effectiveReferenceWhiteEnabled)).map(ToControlState{})}

    , LAGER_QT(referenceWhite){referenceWhiteData}
    , LAGER_QT(referenceWhiteIndex){LAGER_QT(referenceWhite).zoom(referenceWhiteToInt)}
    , LAGER_QT(effectiveReferenceWhite){
        lager::with(
            imageProfileIsRelativeData,
            imageProfileReferenceWhiteData,
            LAGER_QT(referenceWhite)
        ).map(&calcEffectiveReferenceWhite)}
    , LAGER_QT(effectiveReferenceWhiteIndex){LAGER_QT(effectiveReferenceWhite).zoom(referenceWhiteToInt)}
    , LAGER_QT(referenceWhiteState){lager::with(LAGER_QT(effectiveReferenceWhiteIndex), LAGER_QT(effectiveReferenceWhitePresent))
                                        .map(&calcWhiteTypeToComboBoxState)}

    , LAGER_QT(clliPresent){clliPresentData}
    , LAGER_QT(effectiveClliPresent){
        lager::with(
            clliPresentData,
            LAGER_QT(effectiveReferenceWhitePresent)
        ).map(std::logical_and<>{})}
    , LAGER_QT(clliPresentState){lager::with(LAGER_QT(effectiveClliPresent), LAGER_QT(effectiveReferenceWhitePresent)).map(ToControlState{})}

    , LAGER_QT(maxContentLightLevel){
        lager::with(
            clliData[&KisRelativeContentLightLevelInformation::maxContentLightLevel],
            lager::with_setter(LAGER_QT(effectiveReferenceWhite), noopSetter)
        ).zoom(calcRelativeToAbsoluteWhiteLevel)}
    , LAGER_QT(maxFrameAverageLightLevel){
        lager::with(
            clliData[&KisRelativeContentLightLevelInformation::maxFrameAverageLightLevel],
            lager::with_setter(LAGER_QT(effectiveReferenceWhite), noopSetter)
        ).zoom(calcRelativeToAbsoluteWhiteLevel)}

    , LAGER_QT(clliCalculationType){clliData[&KisRelativeContentLightLevelInformation::type]}
    , LAGER_QT(clliCalculationTypeState){lager::with(LAGER_QT(clliCalculationType), LAGER_QT(effectiveClliPresent))
                                             .map(&calcTypeComboBoxState)}

    , LAGER_QT(cviPresent){cviPresentData}

    , LAGER_QT(cviEffectivePresetIndex){cviData.zoom(effectivePresetIndexForCvi)}
    , LAGER_QT(cviEffectivePresetIndexState){
        lager::with(
            LAGER_QT(cviEffectivePresetIndex),
            /// we should explicitly pass the "enabled" value,
            /// since modification of the "state" of the widget
            /// overrides the value set by QGroupBox
            LAGER_QT(cviPresent)
        ).map(&calcCviEffectivePresetIndexState)}

    , LAGER_QT(cviWhiteX){cviData[&KisColorVolumeInformation::white][&KoColorimetryUtils::xy::x]}
    , LAGER_QT(cviWhiteY){cviData[&KisColorVolumeInformation::white][&KoColorimetryUtils::xy::y]}
    , LAGER_QT(cviRedX){cviData[&KisColorVolumeInformation::red][&KoColorimetryUtils::xy::x]}
    , LAGER_QT(cviRedY){cviData[&KisColorVolumeInformation::red][&KoColorimetryUtils::xy::y]}
    , LAGER_QT(cviGreenX){cviData[&KisColorVolumeInformation::green][&KoColorimetryUtils::xy::x]}
    , LAGER_QT(cviGreenY){cviData[&KisColorVolumeInformation::green][&KoColorimetryUtils::xy::y]}
    , LAGER_QT(cviBlueX){cviData[&KisColorVolumeInformation::blue][&KoColorimetryUtils::xy::x]}
    , LAGER_QT(cviBlueY){cviData[&KisColorVolumeInformation::blue][&KoColorimetryUtils::xy::y]}
    , LAGER_QT(cviMaxLuminance){cviData[&KisColorVolumeInformation::maxLuminance]}
    , LAGER_QT(cviMinLuminance){cviData[&KisColorVolumeInformation::minLuminance]}
{
}

KisHDRMetadataModel::~KisHDRMetadataModel() {

}

void KisHDRMetadataModel::setClli(const std::optional<KisRelativeContentLightLevelInformation> &clli)
{
    if (clli) {
        clliPresentData.set(true);
        clliData.set(*clli);
    } else {
        clliPresentData.set(false);
    }
}

std::optional<KisRelativeContentLightLevelInformation> KisHDRMetadataModel::clli() const
{
    const bool enabled = clliPresentData.get();
    if (enabled) {
        return std::make_optional(clliData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setCvi(const std::optional<KisColorVolumeInformation> &cvi)
{
    if (cvi) {
        cviPresentData.set(true);
        cviData.set(*cvi);
    } else {
        cviPresentData.set(false);
        cviData.set(KisColorVolumeInformation{});
    }
}

std::optional<KisColorVolumeInformation> KisHDRMetadataModel::cvi() const
{
    const bool enabled = cviPresentData.get();
    if (enabled) {
        return std::make_optional(cviData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setImageHdrReferenceWhiteMetadata(const std::optional<double> &refWhite)
{
    if (refWhite) {
        referenceWhitePresentData.set(true);
        referenceWhiteData.set(*refWhite);
    } else {
        referenceWhitePresentData.set(false);
        referenceWhiteData.set(203.0);
    }
}

std::optional<double> KisHDRMetadataModel::imageHdrReferenceWhiteMetadata() const
{
    if (referenceWhitePresentData.get() && imageProfileIsRelativeData.get()) {
        return referenceWhiteData.get();
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setImageProfileHdrReferenceWhite(const std::optional<double> &refWhite)
{
    if (refWhite) {
        imageProfileIsRelativeData.set(false);
        imageProfileReferenceWhiteData.set(*refWhite);
    } else {
        imageProfileIsRelativeData.set(true);
        imageProfileReferenceWhiteData.set(203.0);
    }
}
