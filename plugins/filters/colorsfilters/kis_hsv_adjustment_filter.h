/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

    KisFilterConfigurationSP defaultConfiguration() const override;

};


class KisHSVConfigWidget : public KisConfigWidget
{

    Q_OBJECT

public:
    KisHSVConfigWidget(QWidget * parent, Qt::WindowFlags f = 0);
    ~KisHSVConfigWidget() override;

    KisPropertiesConfigurationSP  configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui_WdgHSVAdjustment * m_page;

private Q_SLOTS:

    void configureSliderLimitsAndLabels();
    void resetFilter();
};

#endif
