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
    setShortcut(KShortcut(QKeySequence(Qt::CTRL + Qt::Key_U)));
    setSupportsPainting(true);
    setSupportsIncrementalPainting(false);
}

KisConfigWidget * KisHSVAdjustmentFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisHSVConfigWidget(parent);
}

KoColorTransformation* KisHSVAdjustmentFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
    QHash<QString, QVariant> params;
    if (config) {
        if (config->getBool("colorize")) {
               params["h"] = config->getInt("h", 0.5) / 360.0;
        }
        else {
            params["h"] = config->getInt("h", 0) / 180.0;

        }
        params["s"] = config->getInt("s", 0) * 0.01;
        params["v"] = config->getInt("v", 0) * 0.01;

        params["type"] = config->getInt("type", 1);
        params["colorize"] = config->getBool("colorize", false);
    }
    return cs->createColorTransformation("hsv_adjustment", params);
}

KisFilterConfiguration* KisHSVAdjustmentFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("h", 0);
    config->setProperty("s", 0);
    config->setProperty("v", 0);
    config->setProperty("type", 1);
    config->setProperty("colorize", false);
    return config;
}

KisHSVConfigWidget::KisHSVConfigWidget(QWidget * parent, Qt::WFlags f) : KisConfigWidget(parent, f)
{
    m_page = new Ui_WdgHSVAdjustment();
    m_page->setupUi(this);

    connect(m_page->cmbType, SIGNAL(activated(int)), SLOT(switchType(int)));
    connect(m_page->hue, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->value, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->saturation, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->chkColorize, SIGNAL(toggled(bool)), SLOT(switchColorize(bool)));
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
    c->setProperty("type", m_page->cmbType->currentIndex());
    c->setProperty("colorize", m_page->chkColorize->isChecked());
    return c;
}

void KisHSVConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    m_page->cmbType->setCurrentIndex(config->getInt("type", 1));
    m_page->hue->setValue(config->getInt("h", 0));
    m_page->saturation->setValue(config->getInt("s", 0));
    m_page->value->setValue(config->getInt("v", 0));
    m_page->chkColorize->setChecked(config->getBool("colorize", false));

    switchType(m_page->cmbType->currentIndex());
}

void KisHSVConfigWidget::switchType(int index)
{
    emit sigConfigurationItemChanged();
    switch(index) {
    case 0:
        m_page->label_3->setText(i18n("Value"));
        return;
    case 1:
    default:
        m_page->label_3->setText(i18n("Lightness"));
    }

}

void KisHSVConfigWidget::switchColorize(bool toggle)
{
    if (toggle) {
        m_page->hue->setMinimum(0);
        m_page->hue->setMaximum(360);
        m_page->saturation->setMinimum(0);
        m_page->saturation->setMaximum(100);
        m_page->saturation->setValue(50);
        switchType(1);
    }
    else {
        m_page->hue->setMinimum(-180);
        m_page->hue->setMaximum(180);
        m_page->saturation->setMinimum(-100);
        m_page->saturation->setMaximum(100);

    }
    emit sigConfigurationItemChanged();
}
