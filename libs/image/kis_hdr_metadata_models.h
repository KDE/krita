/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HDR_METADATA_MODELS_H
#define KIS_HDR_METADATA_MODELS_H

#include <QObject>

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <lager/state.hpp>

#include <KisWidgetConnectionUtils.h>
#include <kis_hdr_metadata.h>

#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisHDRMetadataModel : public QObject
{
    Q_OBJECT
public:
    explicit KisHDRMetadataModel(QObject *parent = nullptr);
    ~KisHDRMetadataModel();

    lager::cursor<KisRelativeContentLightLevelInformation> clliData;
    lager::cursor<KisColorVolumeInformation> cviData;
    lager::cursor<double> imageProfileReferenceWhiteData;
    lager::cursor<double> referenceWhiteData;
    lager::cursor<bool> imageProfileIsRelativeData;
    lager::cursor<bool> referenceWhitePresentData;
    lager::cursor<bool> clliPresentData;
    lager::cursor<bool> cviPresentData;

    LAGER_QT_CURSOR(bool, referenceWhitePresent);
    LAGER_QT_READER(bool, effectiveReferenceWhitePresent);
    LAGER_QT_READER(bool, effectiveReferenceWhiteEnabled);
    LAGER_QT_READER(CheckBoxState, referenceWhitePresentState);

    LAGER_QT_CURSOR(double, referenceWhite);
    LAGER_QT_CURSOR(int, referenceWhiteIndex);
    LAGER_QT_READER(double, effectiveReferenceWhite);
    LAGER_QT_READER(int, effectiveReferenceWhiteIndex);
    LAGER_QT_READER(ComboBoxState, referenceWhiteState);

    LAGER_QT_CURSOR(bool, clliPresent);
    LAGER_QT_READER(bool, effectiveClliPresent);
    LAGER_QT_READER(CheckBoxState, clliPresentState);

    LAGER_QT_CURSOR(double, maxContentLightLevel);
    LAGER_QT_CURSOR(double, maxFrameAverageLightLevel);
    LAGER_QT_CURSOR(KisRelativeContentLightLevelInformation::CalculationType, clliCalculationType);
    LAGER_QT_READER(ComboBoxState, clliCalculationTypeState);

    LAGER_QT_CURSOR(bool, cviPresent);

    LAGER_QT_CURSOR(int, cviEffectivePresetIndex);
    LAGER_QT_READER(ComboBoxState, cviEffectivePresetIndexState);

    LAGER_QT_CURSOR(double, cviWhiteX);
    LAGER_QT_CURSOR(double, cviWhiteY);
    LAGER_QT_CURSOR(double, cviRedX);
    LAGER_QT_CURSOR(double, cviRedY);
    LAGER_QT_CURSOR(double, cviGreenX);
    LAGER_QT_CURSOR(double, cviGreenY);
    LAGER_QT_CURSOR(double, cviBlueX);
    LAGER_QT_CURSOR(double, cviBlueY);

    LAGER_QT_CURSOR(double, cviMaxLuminance);
    LAGER_QT_CURSOR(double, cviMinLuminance);

    void setClli(const std::optional<KisRelativeContentLightLevelInformation> &clli);
    void setCvi(const std::optional<KisColorVolumeInformation> &cvi);
    void setImageHdrReferenceWhiteMetadata(const std::optional<double> &refWhite);
    void setImageProfileHdrReferenceWhite(const std::optional<double> &refWhite);

};

#endif // KIS_HDR_METADATA_MODELS_H
