/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
*/


#ifndef _KIS_HSV_ADJUSTMENT_FILTER_H_
#define _KIS_HSV_ADJUSTMENT_FILTER_H_

#include <QList>

#include "filter/kis_filter.h"
#include "kis_config_widget.h"
#include "ui_wdg_hsv_adjustment.h"
#include "filter/kis_color_transformation_filter.h"

class QWidget;
class KoColorTransformation;

/**
 * This class affect Intensity Y of the image
 */
class KisHSVAdjustmentFilter : public KisColorTransformationFilter
{

public:

    KisHSVAdjustmentFilter();

public:

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("hsvadjustment", i18n("HSV/HSL Adjustment"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

};


class KisHSVConfigWidget : public KisConfigWidget
{

    Q_OBJECT

public:
    KisHSVConfigWidget(QWidget * parent, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisHSVConfigWidget() override;

    KisPropertiesConfigurationSP  configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui_WdgHSVAdjustment * m_page;

private Q_SLOTS:

    void configureSliderLimitsAndLabels();
    void resetFilter();
};

#endif
