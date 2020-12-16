/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef DODGE_BURN_H
#define DODGE_BURN_H

#include "filter/kis_color_transformation_filter.h"

#include "kis_config_widget.h"


class KisFilterDodgeBurn : public KisColorTransformationFilter
{
public:
    enum Type {
      SHADOWS,
      MIDTONES,
      HIGHLIGHTS
    };
public:
    KisFilterDodgeBurn(const QString& id, const QString& prefix, const QString& name );
public:

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
private:
    QString m_prefix;
};

class Ui_DodgeBurnConfigurationBaseWidget;

class KisDodgeBurnConfigWidget : public KisConfigWidget
{

public:
    KisDodgeBurnConfigWidget(QWidget * parent, const QString& id);
    ~KisDodgeBurnConfigWidget() override;

    KisPropertiesConfigurationSP  configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    QString m_id;
    Ui_DodgeBurnConfigurationBaseWidget * m_page;
};

#endif
