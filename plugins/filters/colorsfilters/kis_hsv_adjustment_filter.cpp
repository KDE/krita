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

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KoColorProfile.h>

namespace {
struct SliderConfig {
    QString m_text;
    int m_minimum;
    int m_maximum;

    inline void apply(QSpinBox* spinBox, QSlider* slider, QLabel* label) const
    {
        label->setText(m_text);
        slider->setMinimum(m_minimum);
        slider->setMaximum(m_maximum);
        spinBox->setMinimum(m_minimum);
        spinBox->setMaximum(m_maximum);

        int sliderValue = slider->value();
        if (sliderValue < m_minimum || sliderValue > m_maximum) {
            slider->setValue((m_minimum + m_maximum) / 2);
        }
    }

    inline double normalize(int value) const
    {
        return (double)value / (double)m_maximum;
    }

    inline void resetSlider( QSlider* slider) const
    {
        slider->setValue(0);
    }

};

struct WidgetSlidersConfig {
    SliderConfig m_sliders[3];
};

#define PERCENT_FIELD_REL(x) {x, -100, 100}
#define PERCENT_FIELD_ABS(x) {x,    0, 100}
#define DEGREES_FIELD_REL(x) {x, -180, 180}
#define DEGREES_FIELD_ABS(x) {x,    0, 360}
#define HSX_CONFIGS(x) { \
        { {DEGREES_FIELD_REL(i18n("Hue:")), PERCENT_FIELD_REL(i18n("Saturation:")), PERCENT_FIELD_REL(x)} }, \
        { {DEGREES_FIELD_ABS(i18n("Hue:")), PERCENT_FIELD_ABS(i18n("Saturation:")), PERCENT_FIELD_REL(x)} } \
}

const WidgetSlidersConfig WIDGET_CONFIGS[][2] = {
    // Hue/Saturation/Value
    HSX_CONFIGS(i18n("Value:")),
    // Hue/Saturation/Lightness
    HSX_CONFIGS(i18n("Lightness:")),
    // Hue/Saturation/Intensity
    HSX_CONFIGS(i18n("Intensity:")),
    // Hue/Saturation/Luminosity
    HSX_CONFIGS(i18n("Luma:")),
    // Blue Chroma/Red Chroma/Luma
    {{ {PERCENT_FIELD_REL(i18n("Yellow-Blue:")), PERCENT_FIELD_REL(i18n("Green-Red:")), PERCENT_FIELD_REL(i18n("Luma:"))} },
        { {PERCENT_FIELD_ABS(i18n("Yellow-Blue:")), PERCENT_FIELD_ABS(i18n("Green-Red:")), PERCENT_FIELD_REL(i18n("Luma:"))} }}
};

inline const WidgetSlidersConfig& getCurrentWidgetConfig(int type, bool colorize) {
    return WIDGET_CONFIGS[type][colorize ? 1 : 0];
}
}

KisHSVAdjustmentFilter::KisHSVAdjustmentFilter()
        : KisColorTransformationFilter(id(), FiltersCategoryAdjustId, i18n("&HSV Adjustment..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    setSupportsPainting(true);
}

KisConfigWidget * KisHSVAdjustmentFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisHSVConfigWidget(parent);
}

KoColorTransformation* KisHSVAdjustmentFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    QHash<QString, QVariant> params;
    if (config) {
        int type = config->getInt("type", 1);
        bool colorize = config->getBool("colorize", false);
        const WidgetSlidersConfig& widgetConfig = getCurrentWidgetConfig(type, colorize);

        params["h"] = widgetConfig.m_sliders[0].normalize(config->getInt("h", 0));
        params["s"] = widgetConfig.m_sliders[1].normalize(config->getInt("s", 0));
        params["v"] = widgetConfig.m_sliders[2].normalize(config->getInt("v", 0));

        params["type"] = type;
        params["colorize"] = colorize;
        params["lumaRed"]   = cs->lumaCoefficients()[0];
        params["lumaGreen"] = cs->lumaCoefficients()[1];
        params["lumaBlue"]  = cs->lumaCoefficients()[2];
    }
    return cs->createColorTransformation("hsv_adjustment", params);
}

KisFilterConfigurationSP KisHSVAdjustmentFilter::factoryConfiguration() const
{
    KisColorTransformationConfigurationSP config = new KisColorTransformationConfiguration(id().id(), 1);
    config->setProperty("h", 0);
    config->setProperty("s", 0);
    config->setProperty("v", 0);
    config->setProperty("type", 1);
    config->setProperty("colorize", false);
    return config;
}

KisHSVConfigWidget::KisHSVConfigWidget(QWidget * parent, Qt::WindowFlags f) : KisConfigWidget(parent, f)
{
    m_page = new Ui_WdgHSVAdjustment();
    m_page->setupUi(this);

    connect(m_page->cmbType, SIGNAL(activated(int)), this, SLOT(configureSliderLimitsAndLabels()));
    connect(m_page->chkColorize, SIGNAL(toggled(bool)), this, SLOT(configureSliderLimitsAndLabels()));

    connect(m_page->reset,SIGNAL(clicked(bool)),this,SLOT(resetFilter()));

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

KisPropertiesConfigurationSP  KisHSVConfigWidget::configuration() const
{
    KisColorTransformationConfigurationSP c = new KisColorTransformationConfiguration(KisHSVAdjustmentFilter::id().id(), 0);
    c->setProperty("h", m_page->hueSlider->value());
    c->setProperty("s", m_page->saturationSlider->value());
    c->setProperty("v", m_page->valueSlider->value());
    c->setProperty("type", m_page->cmbType->currentIndex());
    c->setProperty("colorize", m_page->chkColorize->isChecked());
    return c;
}

void KisHSVConfigWidget::setConfiguration(const KisPropertiesConfigurationSP  config)
{
    m_page->cmbType->setCurrentIndex(config->getInt("type", 1));
    m_page->chkColorize->setChecked(config->getBool("colorize", false));
    m_page->hueSlider->setValue(config->getInt("h", 0));
    m_page->saturationSlider->setValue(config->getInt("s", 0));
    m_page->valueSlider->setValue(config->getInt("v", 0));
    configureSliderLimitsAndLabels();
}

void KisHSVConfigWidget::configureSliderLimitsAndLabels()
{
    const WidgetSlidersConfig& widget = getCurrentWidgetConfig(m_page->cmbType->currentIndex(), m_page->chkColorize->isChecked());

    widget.m_sliders[0].apply(m_page->hueSpinBox,        m_page->hueSlider,        m_page->label);
    widget.m_sliders[1].apply(m_page->saturationSpinBox, m_page->saturationSlider, m_page->label_2);
    widget.m_sliders[2].apply(m_page->valueSpinBox,      m_page->valueSlider,      m_page->label_3);

    emit sigConfigurationItemChanged();
}

void KisHSVConfigWidget::resetFilter()
{
    const WidgetSlidersConfig& widget = getCurrentWidgetConfig(m_page->cmbType->currentIndex(), m_page->chkColorize->isChecked());

    widget.m_sliders[0].resetSlider(m_page->hueSlider);
    widget.m_sliders[1].resetSlider(m_page->saturationSlider);
    widget.m_sliders[2].resetSlider(m_page->valueSlider);
}

