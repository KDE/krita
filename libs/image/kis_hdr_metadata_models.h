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
    lager::cursor<double> referenceWhiteData;
    lager::cursor<bool> imageProfileRelative;
    lager::cursor<bool> referenceWhiteEnabledData;
    lager::cursor<bool> clliEnabledData;
    lager::cursor<bool> cviEnabledData;

    LAGER_QT_CURSOR(bool, refWhiteEnabled);
    LAGER_QT_READER(CheckBoxState, refWhiteEnabledState);
    LAGER_QT_CURSOR(double, referenceWhite);

    LAGER_QT_CURSOR(bool, clliEnabled);
    LAGER_QT_READER(CheckBoxState, clliEnabledState);
    LAGER_QT_CURSOR(double, maxContentLightLevel);
    LAGER_QT_READER(DoubleSpinBoxState, maxContentLightLevelState);
    LAGER_QT_CURSOR(double, maxFrameAverageLightLevel);
    LAGER_QT_READER(DoubleSpinBoxState, maxFrameAverageLightLevelState);
    LAGER_QT_CURSOR(KisRelativeContentLightLevelInformation::CalculationType, clliCalculationType);
    LAGER_QT_READER(ComboBoxState, clliCalculationTypeState);

    LAGER_QT_CURSOR(bool, cviEnabled);
    LAGER_QT_READER(CheckBoxState, cviEnabledState);
    LAGER_QT_CURSOR(double, cviWhiteX);
    LAGER_QT_CURSOR(double, cviWhiteY);
    LAGER_QT_READER(DoubleSpinBoxState, cviWhiteXState);
    LAGER_QT_READER(DoubleSpinBoxState, cviWhiteYState);
    LAGER_QT_CURSOR(double, cviRedX);
    LAGER_QT_CURSOR(double, cviRedY);
    LAGER_QT_READER(DoubleSpinBoxState, cviRedXState);
    LAGER_QT_READER(DoubleSpinBoxState, cviRedYState);
    LAGER_QT_CURSOR(double, cviGreenX);
    LAGER_QT_CURSOR(double, cviGreenY);
    LAGER_QT_READER(DoubleSpinBoxState, cviGreenXState);
    LAGER_QT_READER(DoubleSpinBoxState, cviGreenYState);
    LAGER_QT_CURSOR(double, cviBlueX);
    LAGER_QT_CURSOR(double, cviBlueY);
    LAGER_QT_READER(DoubleSpinBoxState, cviBlueXState);
    LAGER_QT_READER(DoubleSpinBoxState, cviBlueYState);

    LAGER_QT_CURSOR(double, cviMaxLuminance);
    LAGER_QT_READER(DoubleSpinBoxState, cviMaxLuminanceState);
    LAGER_QT_CURSOR(double, cviMinLuminance);
    LAGER_QT_READER(DoubleSpinBoxState, cviMinLuminanceState);


    void setClli(const std::optional<KisRelativeContentLightLevelInformation> &clli);
    std::optional<KisRelativeContentLightLevelInformation> clli() const;
    void setCvi(const std::optional<KisColorVolumeInformation> &cvi);
    std::optional<KisColorVolumeInformation> cvi() const;
    void setRefWhite(const std::optional<double> &refWhite);
    std::optional<double> refWhite() const;
    void setImageProfileRelative(const std::optional<double> &refWhite);

};

#endif // KIS_HDR_METADATA_MODELS_H
