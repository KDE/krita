/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "DodgeBurn.h"
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>

#include "ui_DodgeBurnConfigurationBaseWidget.h"

KisFilterDodgeBurn::KisFilterDodgeBurn(const QString& id, const QString& prefix, const QString& name ) : KisColorTransformationFilter(KoID(id, name), categoryAdjust(), name), m_prefix(prefix)
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KisConfigWidget * KisFilterDodgeBurn::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);
    return new KisDodgeBurnConfigWidget(parent, id());
}

KoColorTransformation* KisFilterDodgeBurn::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
    QHash<QString, QVariant> params;
    QString suffix = "Midtones";
    if (config) {
        params["exposure"] = config->getDouble("exposure", 0.5);
        int type = config->getInt("type", KisFilterDodgeBurn::MIDTONES);
        switch(type)
          {
            case KisFilterDodgeBurn::HIGHLIGHTS:
              suffix = "Highlights";
              break;
            case KisFilterDodgeBurn::SHADOWS:
              suffix = "Shadows";
              break;
            default:
              break;
          }
    }
    return cs->createColorTransformation(m_prefix + suffix, params);
  
}

KisDodgeBurnConfigWidget::KisDodgeBurnConfigWidget(QWidget * parent, const QString& id) : KisConfigWidget(parent), m_id(id)
{
    m_page = new Ui_DodgeBurnConfigurationBaseWidget();
    m_page->setupUi(this);
    
    connect(m_page->radioButtonHighlights, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->radioButtonMidtones, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->radioButtonShadows, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->sliderExposure, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisDodgeBurnConfigWidget::~KisDodgeBurnConfigWidget()
{
    delete m_page;
}

KisPropertiesConfiguration * KisDodgeBurnConfigWidget::configuration() const
{
    KisFilterConfiguration* c = new KisFilterConfiguration(m_id, 0);
    int type = 0;
    if(m_page->radioButtonHighlights->isChecked())
    {
        type = KisFilterDodgeBurn::HIGHLIGHTS;
    } else if(m_page->radioButtonShadows->isChecked())
    {
        type = KisFilterDodgeBurn::SHADOWS;
    } else {
        type = KisFilterDodgeBurn::MIDTONES;
    }
    c->setProperty("type", type);
    c->setProperty("exposure", m_page->sliderExposure->value() / 100.0);
    return c;
}

void KisDodgeBurnConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    int type = config->getInt("type", KisFilterDodgeBurn::MIDTONES);
    switch(type)
    {
      case KisFilterDodgeBurn::HIGHLIGHTS:
        m_page->radioButtonHighlights->setChecked(true);
        break;
      case KisFilterDodgeBurn::SHADOWS:
        m_page->radioButtonShadows->setChecked(true);
        break;
      default:
      case KisFilterDodgeBurn::MIDTONES:
        m_page->radioButtonMidtones->setChecked(true);
        break;
    }
    m_page->sliderExposure->setValue(config->getDouble("exposure", 0.5) * 100);
}

