/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef KIS_DESATURATE_FILTER_H
#define KIS_DESATURATE_FILTER_H

#include <QObject>
#include <QVariant>
#include <QButtonGroup>

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

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("desaturate", i18n("Desaturate"));
    }

    KisFilterConfigurationSP factoryConfiguration() const override;

};


class KisDesaturateConfigWidget : public KisConfigWidget
{

    Q_OBJECT

public:
    KisDesaturateConfigWidget(QWidget * parent, Qt::WindowFlags f = 0);
    ~KisDesaturateConfigWidget() override;

    KisPropertiesConfigurationSP  configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    Ui_WdgDesaturate *m_page;
    QButtonGroup *m_group;
};


#endif
