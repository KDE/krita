/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef KIS_DESATURATE_FILTER_H
#define KIS_DESATURATE_FILTER_H

#include <QObject>
#include <QVariant>

#include <kis_config_widget.h>
#include <filter/kis_color_transformation_filter.h>

#include "ui_wdg_desaturate.h"

class KoColorSpace;
class KoColorTransformation;

class KisDesaturateFilter : public KisColorTransformationFilter
{
public:
    KisDesaturateFilter();
    ~KisDesaturateFilter() override;
public:

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("desaturate", i18n("Desaturate"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

};


class KisDesaturateConfigWidget : public KisConfigWidget
{

    Q_OBJECT

public:
    KisDesaturateConfigWidget(QWidget * parent, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisDesaturateConfigWidget() override;

    KisPropertiesConfigurationSP  configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui_WdgDesaturate *m_page;
    QButtonGroup *m_group;
};


#endif
