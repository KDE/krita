/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_hdr_metadata_models.h"
#include <KLocalizedString>
#include <lager/constant.hpp>
#include <KisZug.h>
#include <KisLager.h>
namespace {

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

auto multiplyByReferenceWhite = [](double referenceWhite){
    return lager::lenses::getset(
        [referenceWhite] (const double &value) -> double {
            return value * referenceWhite;
        },
        [referenceWhite] (double value, const double &newValue) {
            value = referenceWhite > 0? newValue/referenceWhite: 0.0;
            return value;
        }
        );
};

auto colorimetryXcoord = []() {
    return lager::lenses::getset(
        [] (const KoColorimetryUtils::xy &value) -> double {
            return value.x;
        },
        [] (KoColorimetryUtils::xy value, const double &val) {
            value.x = val;
            return value;
        }
    );
};

auto colorimetryYcoord = []() {
    return lager::lenses::getset(
        [] (const KoColorimetryUtils::xy &value) -> double {
            return value.y;
        },
        [] (KoColorimetryUtils::xy value, const double &val) {
            value.y = val;
            return value;
        }
        );
};

}

using KisWidgetConnectionUtils::ToSpinBoxState;
using KisWidgetConnectionUtils::ToControlState;

KisHDRMetadataModel::KisHDRMetadataModel(QObject *parent)
    : QObject{parent}
    , clliData(lager::make_state(KisRelativeContentLightLevelInformation(), lager::automatic_tag{}))
    , cviData(lager::make_state(KisColorVolumeInformation(), lager::automatic_tag{}))
    , referenceWhiteData(lager::make_state(203.0, lager::automatic_tag{}))
    , imageProfileRelative(lager::make_state(false, lager::automatic_tag{}))
    , referenceWhiteEnabledData(lager::make_state(false, lager::automatic_tag{}))
    , clliEnabledData(lager::make_state(false, lager::automatic_tag{}))
    , cviEnabledData(lager::make_state(false, lager::automatic_tag{}))
    , LAGER_QT(refWhiteEnabled){lager::with(referenceWhiteEnabledData)}
    , LAGER_QT(refWhiteEnabledState){lager::with(LAGER_QT(refWhiteEnabled), lager::make_constant(true)).map(ToControlState{})}
    , LAGER_QT(referenceWhite) {referenceWhiteData}
    , LAGER_QT(clliEnabled){lager::with(clliEnabledData)}
    , LAGER_QT(clliEnabledState){lager::with(LAGER_QT(clliEnabled), lager::make_constant(true)).map(ToControlState{})}
    , LAGER_QT(maxContentLightLevel) {clliData[&KisRelativeContentLightLevelInformation::maxContentLightLevel].zoom(multiplyByReferenceWhite(LAGER_QT(referenceWhite).get()))}
    , LAGER_QT(maxContentLightLevelState) {lager::with(LAGER_QT(maxContentLightLevel),
                                                      lager::make_constant(0.0),
                                                      lager::make_constant(100000.0),
                                                      LAGER_QT(clliEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(maxFrameAverageLightLevel) {clliData[&KisRelativeContentLightLevelInformation::maxFrameAverageLightLevel].zoom(multiplyByReferenceWhite(LAGER_QT(referenceWhite).get()))}
    , LAGER_QT(maxFrameAverageLightLevelState) {lager::with(LAGER_QT(maxFrameAverageLightLevel),
                                                      lager::make_constant(0.0),
                                                      lager::make_constant(100000.0),
                                                      LAGER_QT(clliEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(clliCalculationType) {clliData[&KisRelativeContentLightLevelInformation::type]}
    , LAGER_QT(clliCalculationTypeState) {lager::with(LAGER_QT(clliCalculationType), LAGER_QT(clliEnabled)).map(&calcTypeComboBoxState)}
    , LAGER_QT(cviEnabled){lager::with(cviEnabledData)}
    , LAGER_QT(cviEnabledState){lager::with(LAGER_QT(cviEnabled), lager::make_constant(true)).map(ToControlState{})}
    , LAGER_QT(cviWhiteX){cviData[&KisColorVolumeInformation::white].zoom(colorimetryXcoord())}
    , LAGER_QT(cviWhiteY){cviData[&KisColorVolumeInformation::white].zoom(colorimetryYcoord())}
    , LAGER_QT(cviWhiteXState){lager::with(LAGER_QT(cviWhiteX),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviWhiteYState){lager::with(LAGER_QT(cviWhiteY),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviRedX){cviData[&KisColorVolumeInformation::red].zoom(colorimetryXcoord())}
    , LAGER_QT(cviRedY){cviData[&KisColorVolumeInformation::red].zoom(colorimetryYcoord())}
    , LAGER_QT(cviRedXState){lager::with(LAGER_QT(cviRedX),
                                         lager::make_constant(0.0),
                                         lager::make_constant(1.0),
                                         LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviRedYState){lager::with(LAGER_QT(cviRedY),
                                         lager::make_constant(0.0),
                                         lager::make_constant(1.0),
                                         LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviGreenX){cviData[&KisColorVolumeInformation::green].zoom(colorimetryXcoord())}
    , LAGER_QT(cviGreenY){cviData[&KisColorVolumeInformation::green].zoom(colorimetryYcoord())}
    , LAGER_QT(cviGreenXState){lager::with(LAGER_QT(cviGreenX),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviGreenYState){lager::with(LAGER_QT(cviGreenY),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviBlueX){cviData[&KisColorVolumeInformation::blue].zoom(colorimetryXcoord())}
    , LAGER_QT(cviBlueY){cviData[&KisColorVolumeInformation::blue].zoom(colorimetryYcoord())}
    , LAGER_QT(cviBlueXState){lager::with(LAGER_QT(cviBlueX),
                                          lager::make_constant(0.0),
                                          lager::make_constant(1.0),
                                          LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviBlueYState){lager::with(LAGER_QT(cviBlueY),
                                          lager::make_constant(0.0),
                                          lager::make_constant(1.0),
                                          LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviMaxLuminance){cviData[&KisColorVolumeInformation::maxLuminance]}
    , LAGER_QT(cviMaxLuminanceState){lager::with(LAGER_QT(cviMaxLuminance),
                                                 lager::make_constant(0.0),
                                                 lager::make_constant(10000.0),
                                                 LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}
    , LAGER_QT(cviMinLuminance){cviData[&KisColorVolumeInformation::minLuminance]}
    , LAGER_QT(cviMinLuminanceState){lager::with(LAGER_QT(cviMinLuminance),
                lager::make_constant(0.0),
                lager::make_constant(10000.0),
                LAGER_QT(cviEnabled)).map(ToSpinBoxState{})}

{

}

KisHDRMetadataModel::~KisHDRMetadataModel() {

}

void KisHDRMetadataModel::setClli(const std::optional<KisRelativeContentLightLevelInformation> &clli)
{
    if (clli) {
        clliEnabledData.set(true);
        clliData.set(*clli);
    } else {
        clliEnabledData.set(false);
    }
}

std::optional<KisRelativeContentLightLevelInformation> KisHDRMetadataModel::clli() const
{
    const bool enabled = clliEnabledData.get();
    if (enabled) {
        return std::make_optional(clliData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setCvi(const std::optional<KisColorVolumeInformation> &cvi)
{
    if (cvi) {
        cviEnabledData.set(true);
        cviData.set(*cvi);
    } else {
        cviEnabledData.set(false);
    }
}

std::optional<KisColorVolumeInformation> KisHDRMetadataModel::cvi() const
{
    const bool enabled = cviEnabledData.get();
    if (enabled) {
        return std::make_optional(cviData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setRefWhite(const std::optional<double> &refWhite)
{
    if (refWhite) {
        referenceWhiteEnabledData.set(true);
        referenceWhiteData.set(*refWhite);
    } else {
        referenceWhiteEnabledData.set(false);
    }
}

std::optional<double> KisHDRMetadataModel::refWhite() const
{
    const bool enabled = referenceWhiteEnabledData.get();
    if (enabled) {
        return std::make_optional(referenceWhiteData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setImageProfileRelative(const std::optional<double> &refWhite)
{
    if (refWhite) {
        imageProfileRelative.set(false);
        referenceWhiteData.set(*refWhite);
    } else {
        imageProfileRelative.set(true);
    }
}
