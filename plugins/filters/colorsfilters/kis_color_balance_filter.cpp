/*
 *  Copyright (c) 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kis_color_balance_filter.h"
#include <filter/kis_filter_category_ids.h>
#include "filter/kis_color_transformation_configuration.h"
#include "kis_selection.h"
#include "kis_paint_device.h"
#include "kis_processing_information.h"

KisColorBalanceFilter::KisColorBalanceFilter() 
        : KisColorTransformationFilter(id(), FiltersCategoryAdjustId, i18n("&Color Balance..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
	setSupportsPainting(true);
}

KisConfigWidget * KisColorBalanceFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisColorBalanceConfigWidget(parent);
}

KoColorTransformation * KisColorBalanceFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
	QHash<QString, QVariant> params;
    if (config) {
        params["cyan_red_midtones"] = config->getInt("cyan_red_midtones", 0) * 0.01;
        params["magenta_green_midtones"] = config->getInt("magenta_green_midtones", 0) * 0.01;
        params["yellow_blue_midtones"] = config->getInt("yellow_blue_midtones", 0) * 0.01;

        params["cyan_red_shadows"] = config->getInt("cyan_red_shadows", 0) * 0.01;
        params["magenta_green_shadows"] = config->getInt("magenta_green_shadows", 0) * 0.01;
        params["yellow_blue_shadows"] = config->getInt("yellow_blue_shadows", 0) * 0.01;

        params["cyan_red_highlights"] = config->getInt("cyan_red_highlights", 0) * 0.01;
        params["magenta_green_highlights"] = config->getInt("magenta_green_highlights", 0) * 0.01;
        params["yellow_blue_highlights"] = config->getInt("yellow_blue_highlights", 0) * 0.01;
        params["preserve_luminosity"] = config->getBool("preserve_luminosity", true);

    }
    return cs->createColorTransformation("ColorBalance" , params);
}

KisFilterConfigurationSP KisColorBalanceFilter::factoryConfiguration() const
{
    KisColorTransformationConfigurationSP config = new KisColorTransformationConfiguration(id().id(), 0);
    config->setProperty("cyan_red_midtones", 0);
    config->setProperty("yellow_green_midtones", 0);
    config->setProperty("magenta_blue_midtones", 0);

    config->setProperty("cyan_red_shadows", 0);
    config->setProperty("yellow_green_shadows", 0);
    config->setProperty("magenta_blue_shadows", 0);

    config->setProperty("cyan_red_highlights", 0);
    config->setProperty("yellow_green_highlights", 0);
    config->setProperty("magenta_blue_highlights", 0);
    config->setProperty("preserve_luminosity", true);

    return config;
}

KisColorBalanceConfigWidget::KisColorBalanceConfigWidget(QWidget* parent) : KisConfigWidget(parent)
{
    m_page = new Ui_Form();
    m_page->setupUi(this);

    m_page->cyanRedShadowsSlider->setMaximum(100);
    m_page->cyanRedShadowsSlider->setMinimum(-100);
    m_page->yellowBlueShadowsSlider->setMaximum(100);
    m_page->yellowBlueShadowsSlider->setMinimum(-100);
    m_page->magentaGreenShadowsSlider->setMaximum(100);
    m_page->magentaGreenShadowsSlider->setMinimum(-100);

    m_page->cyanRedMidtonesSlider->setMaximum(100);
    m_page->cyanRedMidtonesSlider->setMinimum(-100);
    m_page->yellowBlueMidtonesSlider->setMaximum(100);
    m_page->yellowBlueMidtonesSlider->setMinimum(-100);
    m_page->magentaGreenMidtonesSlider->setMaximum(100);
    m_page->magentaGreenMidtonesSlider->setMinimum(-100);

    m_page->cyanRedHighlightsSlider->setMaximum(100);
    m_page->cyanRedHighlightsSlider->setMinimum(-100);
    m_page->yellowBlueHighlightsSlider->setMaximum(100);
    m_page->yellowBlueHighlightsSlider->setMinimum(-100);
    m_page->magentaGreenHighlightsSlider->setMaximum(100);
    m_page->magentaGreenHighlightsSlider->setMinimum(-100);

    connect(m_page->cyanRedShadowsSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->magentaGreenShadowsSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->yellowBlueShadowsSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page->cyanRedMidtonesSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->magentaGreenMidtonesSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->yellowBlueMidtonesSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page->cyanRedHighlightsSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->magentaGreenHighlightsSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->yellowBlueHighlightsSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->chkPreserveLuminosity, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page->pushResetShadows, SIGNAL(clicked()), SLOT(slotShadowsClear()));
    connect(m_page->pushResetMidtones, SIGNAL(clicked()), SLOT(slotMidtonesClear()));
    connect(m_page->pushResetHighlights, SIGNAL(clicked()), SLOT(slotHighlightsClear()));

    m_page->cyanRedShadowsSpinbox->setMaximum(100);
    m_page->cyanRedShadowsSpinbox->setMinimum(-100);
    m_page->yellowBlueShadowsSpinbox->setMaximum(100);
    m_page->yellowBlueShadowsSpinbox->setMinimum(-100);
    m_page->magentaGreenShadowsSpinbox->setMaximum(100);
    m_page->magentaGreenShadowsSpinbox->setMinimum(-100);

    m_page->cyanRedMidtonesSpinbox->setMaximum(100);
    m_page->cyanRedMidtonesSpinbox->setMinimum(-100);
    m_page->yellowBlueMidtonesSpinbox->setMaximum(100);
    m_page->yellowBlueMidtonesSpinbox->setMinimum(-100);
    m_page->magentaGreenMidtonesSpinbox->setMaximum(100);
    m_page->magentaGreenMidtonesSpinbox->setMinimum(-100);

    m_page->cyanRedHighlightsSpinbox->setMaximum(100);
    m_page->cyanRedHighlightsSpinbox->setMinimum(-100);
    m_page->yellowBlueHighlightsSpinbox->setMaximum(100);
    m_page->yellowBlueHighlightsSpinbox->setMinimum(-100);
    m_page->magentaGreenHighlightsSpinbox->setMaximum(100);
    m_page->magentaGreenHighlightsSpinbox->setMinimum(-100);

}

KisColorBalanceConfigWidget::~KisColorBalanceConfigWidget()
{
    delete m_page;
}

KisPropertiesConfigurationSP  KisColorBalanceConfigWidget::configuration() const
{
    KisColorTransformationConfigurationSP c = new KisColorTransformationConfiguration(KisColorBalanceFilter::id().id(), 0);

    c->setProperty("cyan_red_shadows", m_page->cyanRedShadowsSlider->value());
    c->setProperty("magenta_green_shadows", m_page->magentaGreenShadowsSlider->value());
    c->setProperty("yellow_blue_shadows", m_page->yellowBlueShadowsSlider->value());

    c->setProperty("cyan_red_midtones", m_page->cyanRedMidtonesSlider->value());
    c->setProperty("magenta_green_midtones", m_page->magentaGreenMidtonesSlider->value());
    c->setProperty("yellow_blue_midtones", m_page->yellowBlueMidtonesSlider->value());

    c->setProperty("cyan_red_highlights", m_page->cyanRedHighlightsSlider->value());
    c->setProperty("magenta_green_highlights", m_page->magentaGreenHighlightsSlider->value());
    c->setProperty("yellow_blue_highlights", m_page->yellowBlueHighlightsSlider->value());

    c->setProperty("preserve_luminosity", m_page->chkPreserveLuminosity->isChecked());
    return c;
}

void KisColorBalanceConfigWidget::setConfiguration(const KisPropertiesConfigurationSP  config)
{
    m_page->cyanRedMidtonesSlider->setValue( config->getDouble("cyan_red_midtones", 0));
    m_page->magentaGreenMidtonesSlider->setValue( config->getDouble("magenta_green_midtones", 0));
    m_page->yellowBlueMidtonesSlider->setValue( config->getDouble("yellow_blue_midtones", 0));

    m_page->cyanRedShadowsSlider->setValue( config->getDouble("cyan_red_shadows", 0));
    m_page->magentaGreenShadowsSlider->setValue( config->getDouble("magenta_green_shadows", 0));
    m_page->yellowBlueShadowsSlider->setValue( config->getDouble("yellow_blue_shadows", 0));

    m_page->cyanRedHighlightsSlider->setValue( config->getDouble("cyan_red_highlights", 0));
    m_page->magentaGreenHighlightsSlider->setValue( config->getDouble("magenta_green_highlights", 0));
    m_page->yellowBlueHighlightsSlider->setValue( config->getDouble("yellow_blue_highlights", 0));
    m_page->chkPreserveLuminosity->setChecked(config->getBool("preserve_luminosity", true));
}

void KisColorBalanceConfigWidget::slotMidtonesClear()
{
    m_page->cyanRedMidtonesSlider->setValue(0);
    m_page->magentaGreenMidtonesSlider->setValue(0);
    m_page->yellowBlueMidtonesSlider->setValue(0);
}

void KisColorBalanceConfigWidget::slotHighlightsClear()
{
    m_page->cyanRedHighlightsSlider->setValue(0);
    m_page->magentaGreenHighlightsSlider->setValue(0);
    m_page->yellowBlueHighlightsSlider->setValue(0);
}

void KisColorBalanceConfigWidget::slotShadowsClear()
{
    m_page->cyanRedShadowsSlider->setValue(0);
    m_page->magentaGreenShadowsSlider->setValue(0);
    m_page->yellowBlueShadowsSlider->setValue(0);
}
