/*
 *  Copyright (c) 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License, or (at your option) any later version.
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

#include "kis_color_balance_filter.h"
#include <KoProgressUpdater.h>
#include "filter/kis_filter_configuration.h"
#include "kis_selection.h"
#include "kis_paint_device.h"
#include "kis_processing_information.h"

KisColorBalanceFilter::KisColorBalanceFilter() 
		: KisColorTransformationFilter(id(), categoryAdjust(), i18n("&Color Balance.."))
{
	setShortcut(KShortcut(QKeySequence(Qt::CTRL + Qt::Key_C)));
	setSupportsPainting(true);
	setSupportsIncrementalPainting(false);
}

KisConfigWidget * KisColorBalanceFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisColorBalanceConfigWidget(parent);
}

KoColorTransformation * KisColorBalanceFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
	QHash<QString, QVariant> params;
	QString suffix = "Midtones";
	if (config) {
        params["cyan_red"] = config->getInt("cyan_red", 0) * 0.01;
        params["magenta_green"] = config->getInt("magenta_green", 0) * 0.01;
        params["yellow_blue"] = config->getInt("yellow_blue", 0) * 0.01;
        params["preserve_luminosity"] = config->getBool("preserve_luminosity", true);
        int type = config->getInt("type", KisColorBalanceFilter::MIDTONES);
        switch(type)
        {
            case KisColorBalanceFilter::HIGHLIGHTS:
                suffix = "Highlights";
                break;
            case KisColorBalanceFilter::SHADOWS:
                suffix = "Shadows";
                break;
            default: break;
        }
    }
    return cs->createColorTransformation("Balance" + suffix , params);
}

KisFilterConfiguration* KisColorBalanceFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 0);
    config->setProperty("cyan_red", 0);
    config->setProperty("yellow_green", 0);
    config->setProperty("magenta_blue", 0);
    config->setProperty("preserve_luminosity", true);
    return config;
}

KisColorBalanceConfigWidget::KisColorBalanceConfigWidget(QWidget* parent) : KisConfigWidget(parent)
{
    m_page = new Ui_Form();
    m_page->setupUi(this);
    m_page->cyanRedSlider->setMaximum(100);
    m_page->cyanRedSlider->setMinimum(-100);
    m_page->cyanRedSpinbox->setMaximum(100);
    m_page->cyanRedSpinbox->setMinimum(-100);
    m_page->yellowBlueSlider->setMaximum(100);
    m_page->yellowBlueSlider->setMinimum(-100);
    m_page->yellowBlueSpinbox->setMaximum(100);
    m_page->yellowBlueSpinbox->setMinimum(-100);
    m_page->magentaGreenSlider->setMaximum(100);
    m_page->magentaGreenSlider->setMinimum(-100);
    m_page->magentaGreenSpinbox->setMaximum(100);
    m_page->magentaGreenSpinbox->setMinimum(-100);

    connect(m_page->radioButtonMidtones, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->radioButtonHighlights, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->radioButtonShadows, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    \
    connect(m_page->cyanRedSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->magentaGreenSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->yellowBlueSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->chkPreserve, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
}

KisColorBalanceConfigWidget::~KisColorBalanceConfigWidget()
{
    delete m_page;
}

KisPropertiesConfiguration * KisColorBalanceConfigWidget::configuration() const
{
    KisFilterConfiguration* c = new KisFilterConfiguration(KisColorBalanceFilter::id().id(), 0);
    int type = 0;
    if(m_page->radioButtonHighlights->isChecked()) {
        type = KisColorBalanceFilter::HIGHLIGHTS;
    } else if(m_page->radioButtonShadows->isChecked()) {
        type = KisColorBalanceFilter::SHADOWS;
    } else {
        type = KisColorBalanceFilter::MIDTONES;
    }
    c->setProperty("type", type);
    c->setProperty("cyan_red", m_page->cyanRedSlider->value());
    c->setProperty("magenta_green", m_page->magentaGreenSlider->value());
    c->setProperty("yellow_blue", m_page->yellowBlueSlider->value());
    c->setProperty("preserve_luminosity", m_page->chkPreserve->isChecked());
    return c;
}

void KisColorBalanceConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    int type = config->getInt("type", KisColorBalanceFilter::MIDTONES);
    switch(type)
    {
        case KisColorBalanceFilter::HIGHLIGHTS:
            m_page->radioButtonHighlights->setChecked(true);
            break;
        case KisColorBalanceFilter::SHADOWS:
            m_page->radioButtonShadows->setChecked(true);
            break;
        default:
        case KisColorBalanceFilter::MIDTONES:
            m_page->radioButtonMidtones->setChecked(true);
            break;
    }
    m_page->cyanRedSlider->setValue( config->getDouble("cyan_red", 0));
    m_page->magentaGreenSlider->setValue( config->getDouble("magenta_green", 0));
    m_page->yellowBlueSlider->setValue( config->getDouble("yellow_blue", 0));
    m_page->chkPreserve->setChecked(config->getBool("preserve_luminosity", true));
}
