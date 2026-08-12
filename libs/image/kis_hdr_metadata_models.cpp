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

QList<double> refWhiteList {
    80.0,
    203.0
};

auto referenceWhiteToInt = [](){
    return lager::lenses::getset(
        [] (const double &value) -> int {
            int index = -1;
            for (int i = 0; i < refWhiteList.size(); i++) {
                if (qFuzzyCompare(value, refWhiteList.at(i))) {
                    index = i;
                    break;
                }
            }
            return index;
        },
        [] (double value, const int &newValue) {
            value = refWhiteList.value(newValue, 0.0);
            return value;
        }
        );
};

ComboBoxState whiteTypeToComboBoxState(int index, bool enabled)
{
    QStringList values;
    QStringList toolTips;

    for (int i = 0; i < refWhiteList.size(); i++) {
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
    , imageProfileRelativeData(lager::make_state(false, lager::automatic_tag{}))
    , referenceWhiteCheckedData(lager::make_state(false, lager::automatic_tag{}))
    , clliCheckedData(lager::make_state(false, lager::automatic_tag{}))
    , cviCheckedData(lager::make_state(false, lager::automatic_tag{}))
    , LAGER_QT(refWhiteChecked){lager::with(referenceWhiteCheckedData)}
    , LAGER_QT(refWhiteEnabled){lager::with(imageProfileRelativeData)}
    , LAGER_QT(refWhiteCheckedState){lager::with(LAGER_QT(refWhiteChecked), LAGER_QT(refWhiteEnabled)).map(ToControlState{})}
    , LAGER_QT(referenceWhite) {referenceWhiteData}
    , LAGER_QT(referenceWhiteIndex) {LAGER_QT(referenceWhite).zoom(referenceWhiteToInt())}
    , LAGER_QT(referenceWhiteState) {lager::with(LAGER_QT(referenceWhiteIndex), LAGER_QT(refWhiteChecked)).map(&whiteTypeToComboBoxState)}
    , LAGER_QT(clliChecked){lager::with(clliCheckedData)}
    , LAGER_QT(clliCheckedState){lager::with(LAGER_QT(clliChecked), lager::make_constant(true)).map(ToControlState{})}
    , LAGER_QT(maxContentLightLevel) {clliData[&KisRelativeContentLightLevelInformation::maxContentLightLevel].zoom(multiplyByReferenceWhite(LAGER_QT(referenceWhite).get()))}
    , LAGER_QT(maxContentLightLevelState) {lager::with(LAGER_QT(maxContentLightLevel),
                                                      lager::make_constant(0.0),
                                                      lager::make_constant(100000.0),
                                                      LAGER_QT(clliChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(maxFrameAverageLightLevel) {clliData[&KisRelativeContentLightLevelInformation::maxFrameAverageLightLevel].zoom(multiplyByReferenceWhite(LAGER_QT(referenceWhite).get()))}
    , LAGER_QT(maxFrameAverageLightLevelState) {lager::with(LAGER_QT(maxFrameAverageLightLevel),
                                                      lager::make_constant(0.0),
                                                      lager::make_constant(100000.0),
                                                      LAGER_QT(clliChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(clliCalculationType) {clliData[&KisRelativeContentLightLevelInformation::type]}
    , LAGER_QT(clliCalculationTypeState) {lager::with(LAGER_QT(clliCalculationType), LAGER_QT(clliChecked)).map(&calcTypeComboBoxState)}
    , LAGER_QT(cviChecked){lager::with(cviCheckedData)}
    , LAGER_QT(cviCheckedState){lager::with(LAGER_QT(cviChecked), lager::make_constant(true)).map(ToControlState{})}
    , LAGER_QT(cviWhiteX){cviData[&KisColorVolumeInformation::white].zoom(colorimetryXcoord())}
    , LAGER_QT(cviWhiteY){cviData[&KisColorVolumeInformation::white].zoom(colorimetryYcoord())}
    , LAGER_QT(cviWhiteXState){lager::with(LAGER_QT(cviWhiteX),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviWhiteYState){lager::with(LAGER_QT(cviWhiteY),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviRedX){cviData[&KisColorVolumeInformation::red].zoom(colorimetryXcoord())}
    , LAGER_QT(cviRedY){cviData[&KisColorVolumeInformation::red].zoom(colorimetryYcoord())}
    , LAGER_QT(cviRedXState){lager::with(LAGER_QT(cviRedX),
                                         lager::make_constant(0.0),
                                         lager::make_constant(1.0),
                                         LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviRedYState){lager::with(LAGER_QT(cviRedY),
                                         lager::make_constant(0.0),
                                         lager::make_constant(1.0),
                                         LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviGreenX){cviData[&KisColorVolumeInformation::green].zoom(colorimetryXcoord())}
    , LAGER_QT(cviGreenY){cviData[&KisColorVolumeInformation::green].zoom(colorimetryYcoord())}
    , LAGER_QT(cviGreenXState){lager::with(LAGER_QT(cviGreenX),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviGreenYState){lager::with(LAGER_QT(cviGreenY),
                                           lager::make_constant(0.0),
                                           lager::make_constant(1.0),
                                           LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviBlueX){cviData[&KisColorVolumeInformation::blue].zoom(colorimetryXcoord())}
    , LAGER_QT(cviBlueY){cviData[&KisColorVolumeInformation::blue].zoom(colorimetryYcoord())}
    , LAGER_QT(cviBlueXState){lager::with(LAGER_QT(cviBlueX),
                                          lager::make_constant(0.0),
                                          lager::make_constant(1.0),
                                          LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviBlueYState){lager::with(LAGER_QT(cviBlueY),
                                          lager::make_constant(0.0),
                                          lager::make_constant(1.0),
                                          LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviMaxLuminance){cviData[&KisColorVolumeInformation::maxLuminance]}
    , LAGER_QT(cviMaxLuminanceState){lager::with(LAGER_QT(cviMaxLuminance),
                                                 lager::make_constant(0.0),
                                                 lager::make_constant(10000.0),
                                                 LAGER_QT(cviChecked)).map(ToSpinBoxState{})}
    , LAGER_QT(cviMinLuminance){cviData[&KisColorVolumeInformation::minLuminance]}
    , LAGER_QT(cviMinLuminanceState){lager::with(LAGER_QT(cviMinLuminance),
                lager::make_constant(0.0),
                lager::make_constant(10000.0),
                LAGER_QT(cviChecked)).map(ToSpinBoxState{})}

{

}

KisHDRMetadataModel::~KisHDRMetadataModel() {

}

void KisHDRMetadataModel::setClli(const std::optional<KisRelativeContentLightLevelInformation> &clli)
{
    if (clli) {
        clliCheckedData.set(true);
        clliData.set(*clli);
    } else {
        clliCheckedData.set(false);
    }
}

std::optional<KisRelativeContentLightLevelInformation> KisHDRMetadataModel::clli() const
{
    const bool enabled = clliCheckedData.get();
    if (enabled) {
        return std::make_optional(clliData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setCvi(const std::optional<KisColorVolumeInformation> &cvi)
{
    if (cvi) {
        cviCheckedData.set(true);
        cviData.set(*cvi);
    } else {
        cviCheckedData.set(false);
    }
}

std::optional<KisColorVolumeInformation> KisHDRMetadataModel::cvi() const
{
    const bool enabled = cviCheckedData.get();
    if (enabled) {
        return std::make_optional(cviData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setRefWhite(const std::optional<double> &refWhite)
{
    if (refWhite) {
        referenceWhiteCheckedData.set(true);
        referenceWhiteData.set(*refWhite);
    } else {
        referenceWhiteCheckedData.set(false);
    }
}

std::optional<double> KisHDRMetadataModel::refWhite() const
{
    const bool enabled = referenceWhiteCheckedData.get();
    if (enabled) {
        return std::make_optional(referenceWhiteData.get());
    }
    return std::nullopt;
}

void KisHDRMetadataModel::setImageProfileRelative(const std::optional<double> &refWhite)
{
    if (refWhite) {
        imageProfileRelativeData.set(false);
        referenceWhiteData.set(*refWhite);
    } else {
        imageProfileRelativeData.set(true);
    }
}
