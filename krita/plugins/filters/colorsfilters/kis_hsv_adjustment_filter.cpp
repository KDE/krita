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

#include "kis_hsv_adjustment_filter.h"


#include <KoProgressUpdater.h>

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

KisHSVAdjustmentFilter::KisHSVAdjustmentFilter()
        : KisColorTransformationFilter(id(), categoryAdjust(), i18n("&HSV Adjustment..."))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KisConfigWidget * KisHSVAdjustmentFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);
    return new KisHSVConfigWidget(parent);
}

KoColorTransformation* KisHSVAdjustmentFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
    QHash<QString, QVariant> params;
    if (config) {
        params["h"] = config->getInt("h", 0) / 180.0;
        params["s"] = config->getInt("s", 0) * 0.01;
        params["v"] = config->getInt("v", 0) * 0.01;
    }
    return cs->createColorTransformation("hsv_adjustment", params);
}

KisFilterConfiguration* KisHSVAdjustmentFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("h", 0);
    config->setProperty("s", 0);
    config->setProperty("v", 0);
    return config;
}

KisHSVConfigWidget::KisHSVConfigWidget(QWidget * parent, Qt::WFlags f) : KisConfigWidget(parent, f)
{
    m_page = new Ui_WdgHSVAdjustment();
    m_page->setupUi(this);
    connect(m_page->hue, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->value, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->saturation, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisHSVConfigWidget::~KisHSVConfigWidget()
{
    delete m_page;
}

KisPropertiesConfiguration * KisHSVConfigWidget::configuration() const
{
    KisFilterConfiguration* c = new KisFilterConfiguration(KisHSVAdjustmentFilter::id().id(), 0);
    c->setProperty("h", m_page->hue->value());
    c->setProperty("s", m_page->saturation->value());
    c->setProperty("v", m_page->value->value());
    return c;
}

void KisHSVConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    m_page->hue->setValue(config->getInt("h", 0));
    m_page->saturation->setValue(config->getInt("s", 0));
    m_page->value->setValue(config->getInt("v", 0));
}
