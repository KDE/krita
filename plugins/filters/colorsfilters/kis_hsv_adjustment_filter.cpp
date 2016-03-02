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

#include <filter/kis_color_transformation_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KoColorProfile.h>

KisHSVAdjustmentFilter::KisHSVAdjustmentFilter()
        : KisColorTransformationFilter(id(), categoryAdjust(), i18n("&HSV Adjustment..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    setSupportsPainting(true);
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
               params["h"] = config->getDouble("h", 0.5) / 360.0;
        }
        else {
            params["h"] = config->getDouble("h", 0) / 180.0;

        }
        params["s"] = config->getInt("s", 0) * 0.01;
        params["v"] = config->getInt("v", 0) * 0.01;

        params["type"] = config->getInt("type", 1);
        params["colorize"] = config->getBool("colorize", false);
        params["lumaRed"]   = cs->lumaCoefficients()[0];
        params["lumaGreen"] = cs->lumaCoefficients()[1];
        params["lumaBlue"]  = cs->lumaCoefficients()[2];
    }
    return cs->createColorTransformation("hsv_adjustment", params);
}

KisFilterConfiguration* KisHSVAdjustmentFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisColorTransformationConfiguration* config = new KisColorTransformationConfiguration(id().id(), 1);
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

    m_page->hueSlider->setRange(-180, 180);
    m_page->hueSlider->setValue(0);
    m_page->hueSpinBox->setRange(-180, 180);
    m_page->hueSpinBox->setValue(0);


    m_page->saturationSlider->setRange(-100, 100);
    m_page->saturationSlider->setValue(0);
    m_page->saturationSpinBox->setRange(-100, 100);
    m_page->saturationSpinBox->setValue(0);



    m_page->valueSlider->setRange(-100, 100);
    m_page->valueSlider->setValue(0);
    m_page->valueSpinBox->setRange(-100, 100);
    m_page->valueSpinBox->setValue(0);

    connect(m_page->cmbType, SIGNAL(activated(int)), SLOT(switchType(int)));
    connect(m_page->chkColorize, SIGNAL(toggled(bool)), SLOT(switchColorize(bool)));


    // connect horizontal sliders
    connect(m_page->hueSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->saturationSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->valueSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page->hueSpinBox, SIGNAL(valueChanged(int)), m_page->hueSlider, SLOT(setValue(int)));
    connect(m_page->saturationSpinBox, SIGNAL(valueChanged(int)), m_page->saturationSlider, SLOT(setValue(int)));
    connect(m_page->valueSpinBox, SIGNAL(valueChanged(int)), m_page->valueSlider, SLOT(setValue(int)));

    connect(m_page->hueSlider, SIGNAL(valueChanged(int)), m_page->hueSpinBox, SLOT(setValue(int)));
    connect(m_page->saturationSlider, SIGNAL(valueChanged(int)), m_page->saturationSpinBox, SLOT(setValue(int)));
    connect(m_page->valueSlider, SIGNAL(valueChanged(int)), m_page->valueSpinBox, SLOT(setValue(int)));

}

KisHSVConfigWidget::~KisHSVConfigWidget()
{
    delete m_page;
}

KisPropertiesConfiguration * KisHSVConfigWidget::configuration() const
{
    KisColorTransformationConfiguration* c = new KisColorTransformationConfiguration(KisHSVAdjustmentFilter::id().id(), 0);
    c->setProperty("h", m_page->hueSlider->value());
    c->setProperty("s", m_page->saturationSlider->value());
    c->setProperty("v", m_page->valueSlider->value());
    c->setProperty("type", m_page->cmbType->currentIndex());
    c->setProperty("colorize", m_page->chkColorize->isChecked());
    return c;
}

void KisHSVConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    m_page->cmbType->setCurrentIndex(config->getInt("type", 1));
    m_page->hueSlider->setValue(config->getInt("h", 0));
    m_page->saturationSlider->setValue(config->getInt("s", 0));
    m_page->valueSlider->setValue(config->getInt("v", 0));
    m_page->chkColorize->setChecked(config->getBool("colorize", false));
    switchType(m_page->cmbType->currentIndex());
}

void KisHSVConfigWidget::switchType(int index)
{
    emit sigConfigurationItemChanged();
    m_page->label->setText(i18n("Hue:"));
    m_page->label_2->setText(i18n("Saturation:"));
    m_page->hueSlider->setMinimum(-180);
    m_page->hueSlider->setMaximum(180);
    switch(index) {
    case 0:
        m_page->label_3->setText(i18n("Value:"));
        return;
    case 1:
        m_page->label_3->setText(i18n("Lightness:"));
        return;
    case 2:
        m_page->label_3->setText(i18n("Intensity:"));
        return;
    case 3:
        m_page->label_3->setText(i18n("Luma:"));
        return;
    case 4:
        m_page->label->setText(i18n("Yellow-Blue:"));
        m_page->label_2->setText(i18n("Green-Red:"));
        m_page->label_3->setText(i18n("Luma:"));
        m_page->hueSlider->setRange(-100, 100);
        m_page->hueSlider->setValue(0);
    default:
        m_page->label_3->setText(i18n("Lightness:"));
    }
    

}

void KisHSVConfigWidget::switchColorize(bool toggle)
{
    if (toggle) {
        m_page->hueSlider->setMinimum(0);
        m_page->hueSlider->setMaximum(360);

        m_page->saturationSlider->setMinimum(0);
        m_page->saturationSlider->setMaximum(100);

        if (m_page->saturationSlider->value() < m_page->saturationSlider->minimum() || m_page->saturationSlider->value() > m_page->saturationSlider->maximum()) {
            m_page->saturationSlider->setValue(50);
        }
        switchType(1);
    }
    else {
        m_page->hueSlider->setMinimum(-180);
        m_page->hueSlider->setMaximum(180);
        m_page->saturationSlider->setMinimum(-100);
        m_page->saturationSlider->setMaximum(100);

    }
    emit sigConfigurationItemChanged();
}
