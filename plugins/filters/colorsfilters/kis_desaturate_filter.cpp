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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_desaturate_filter.h"


#include <math.h>

#include <stdlib.h>
#include <string.h>

#include <QSlider>
#include <QPoint>
#include <QColor>
#include <QButtonGroup>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include "KoBasicHistogramProducers.h"
#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_selection.h>
#include "filter/kis_filter_registry.h"
#include <kis_painter.h>
#include <KoColorSpaceConstants.h>
#include <KoCompositeOp.h>
#include <kis_iterator_ng.h>

KisDesaturateFilter::KisDesaturateFilter()
   : KisColorTransformationFilter(id(), FiltersCategoryAdjustId, i18n("&Desaturate..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_U));
    setSupportsPainting(true);
}

KisDesaturateFilter::~KisDesaturateFilter()
{
}

KisConfigWidget *KisDesaturateFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisDesaturateConfigWidget(parent);
}


KoColorTransformation* KisDesaturateFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    QHash<QString, QVariant> params;
    if (config) {
        params["type"] = config->getInt("type", 0);
    }
    return  cs->createColorTransformation("desaturate_adjustment", params);
}

KisFilterConfigurationSP KisDesaturateFilter::factoryConfiguration() const
{
    KisColorTransformationConfigurationSP config = new KisColorTransformationConfiguration(id().id(), 1);
    config->setProperty("type", 0);
    return config;
}

KisDesaturateConfigWidget::KisDesaturateConfigWidget(QWidget * parent, Qt::WindowFlags f) : KisConfigWidget(parent, f)
{
    m_page = new Ui_WdgDesaturate();
    m_page->setupUi(this);
    m_group = new QButtonGroup(this);
    m_group->addButton(m_page->radioLightness, 0);
    m_group->addButton(m_page->radioLuminosityBT709, 1);
    m_group->addButton(m_page->radioLuminosityBT601, 2);
    m_group->addButton(m_page->radioAverage, 3);
    m_group->addButton(m_page->radioMin, 4);
    m_group->addButton(m_page->radioMax, 5);
    m_group->setExclusive(true);
    connect(m_group, SIGNAL(buttonClicked(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisDesaturateConfigWidget::~KisDesaturateConfigWidget()
{
    delete m_page;
}

KisPropertiesConfigurationSP  KisDesaturateConfigWidget::configuration() const
{
    KisColorTransformationConfigurationSP c = new KisColorTransformationConfiguration(KisDesaturateFilter::id().id(), 0);
    c->setProperty("type", m_group->checkedId());
    return c;
}

void KisDesaturateConfigWidget::setConfiguration(const KisPropertiesConfigurationSP  config)
{
    m_group->button(config->getInt("type", 0))->setChecked(true);
    emit sigConfigurationItemChanged();
}
