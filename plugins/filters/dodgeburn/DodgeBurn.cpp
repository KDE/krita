/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DodgeBurn.h"
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_paint_device.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_DodgeBurnConfigurationBaseWidget.h"

KisFilterDodgeBurn::KisFilterDodgeBurn(const QString& id, const QString& prefix, const QString& name ) : KisColorTransformationFilter(KoID(id, name), FiltersCategoryAdjustId, name), m_prefix(prefix)
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

KisConfigWidget * KisFilterDodgeBurn::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisDodgeBurnConfigWidget(parent, id());
}

KoColorTransformation* KisFilterDodgeBurn::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
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

KisPropertiesConfigurationSP  KisDodgeBurnConfigWidget::configuration() const
{
    KisColorTransformationConfigurationSP c = new KisColorTransformationConfiguration(m_id, 0, KisGlobalResourcesInterface::instance());
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

void KisDodgeBurnConfigWidget::setConfiguration(const KisPropertiesConfigurationSP  config)
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

