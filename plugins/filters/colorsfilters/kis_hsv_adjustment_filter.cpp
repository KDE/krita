/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 Sam Linnfer <littlelightlittlefire@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
*/

#include <array>

#include "kis_hsv_adjustment_filter.h"

#include <klocalizedstring.h>

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorConversions.h>
#include <KisGlobalResourcesInterface.h>
#include <KisHsvColorSlider.h>

#include "kis_signals_blocker.h"

namespace {

enum class SLIDER_TYPE {
    HUE,
    SATURATION,
    VALUE,
    LIGHTNESS,
    LUMA,
    INTENSITY,
    YELLOW_BLUE,
    GREEN_RED,
    LUMA_YUV,
};

// Corresponds the 5 transformations defined in kis_hsv_adjustment.cpp
// 0:HSV, 1:HSL, 2:HSI, 3:HSY, 4:YUV
enum class SLIDER_SET {
    HSV, HSL, HSI, HSY, YUV,
};

struct SliderSettings {
    SliderSettings(SLIDER_TYPE type, KisHsvColorSlider::MIX_MODE mixMode, const KLocalizedString &title, int min, int max, int minRelative, int maxRelative, int resetValue)
        : m_type(type)
        , m_mixMode(mixMode)
        , m_title(title)
        , m_min(min)
        , m_max(max)
        , m_minRelative(minRelative)
        , m_maxRelative(maxRelative)
        , m_resetValue(resetValue)
    {
    }

    void apply(QLabel *label, KisHsvColorSlider *slider, QSpinBox *spinBox, bool prevColorize, bool colorize) const {
        int value = slider->value();
        const double norm = normalize(prevColorize, value);
        const int min = colorize ? m_min : m_minRelative;
        const int max = colorize ? m_max : m_maxRelative;

        label->setText(m_title.toString());
        slider->setMinimum(min);
        slider->setMaximum(max);
        spinBox->setMinimum(min);
        spinBox->setMaximum(max);

        if (prevColorize != colorize) {
            // Rescale the value
            value = static_cast<int>(min + norm * static_cast<double>(max - min));
        }

        value = qBound(min, value, max);
        spinBox->setValue(value);
        slider->setValue(value);
    }

    void recolor(KisHsvColorSlider *slider, SLIDER_SET set, bool colorize, qreal nHue, qreal nSaturation, qreal nValue) {
        slider->setMixMode(m_mixMode);
        SLIDER_TYPE type = m_type;

        if (!colorize) {
            switch (m_type) {
            case SLIDER_TYPE::HUE:
                // Slider colors shift to mimic hue changes on canvas.
                nHue = fmod(nHue + 0.5, 1);
                break;
            case SLIDER_TYPE::GREEN_RED: // fallthrough
            case SLIDER_TYPE::YELLOW_BLUE:
                nHue = 0.5;
                nSaturation = 0.5;
                nValue = 0.5;
                break;

            default:
                nHue = 0;
                nSaturation = 0;
                nValue = 1;

                // Display a gray bar for YUV Luma in non-colorize mode
                if (m_type == SLIDER_TYPE::LUMA_YUV) {
                    type = SLIDER_TYPE::LUMA;
                }
            }
        }

        switch (type) {
        case SLIDER_TYPE::HUE: {
            // The hue slider does not move on colorize mode
            if (colorize) {
                nHue = 0;
            }
            slider->setColors(nHue, 1, 1, nHue, 1, 1);
            slider->setCircularHue(true);
            break;
        }

        case SLIDER_TYPE::SATURATION: {
            if (colorize) {
                switch (set) {
                case SLIDER_SET::HSY:
                    slider->setMixMode(KisHsvColorSlider::MIX_MODE::HSY);
                    break;

                case SLIDER_SET::HSI:
                    slider->setMixMode(KisHsvColorSlider::MIX_MODE::HSI);
                    break;

                case SLIDER_SET::HSL:
                    slider->setMixMode(KisHsvColorSlider::MIX_MODE::HSL);
                    break;

                default: // fallthrough
                case SLIDER_SET::HSV:
                    break;
                }
            }

            slider->setColors(nHue, 0, nValue, nHue, 1, nValue);
            break;
        }

        case SLIDER_TYPE::VALUE:     // fallthrough
        case SLIDER_TYPE::LIGHTNESS: // fallthrough
        case SLIDER_TYPE::LUMA:      // fallthrough
        case SLIDER_TYPE::INTENSITY: {
            slider->setColors(nHue, nSaturation, 0, nHue, nSaturation, 1);
            break;
        }

        case SLIDER_TYPE::GREEN_RED:
        case SLIDER_TYPE::YELLOW_BLUE:
        case SLIDER_TYPE::LUMA_YUV: {
            const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(YCbCrAColorModelID.id(), Integer8BitsColorDepthID.id());

            qreal minR, minG, minB;
            qreal maxR, maxG, maxB;

            if (m_type == SLIDER_TYPE::GREEN_RED) { // Cr
                YUVToRGB(nValue, nHue, 0, &minR, &minG, &minB);
                YUVToRGB(nValue, nHue, 1, &maxR, &maxG, &maxB);
            } else if (m_type == SLIDER_TYPE::YELLOW_BLUE) { // Cb
                YUVToRGB(nValue, 0, nSaturation, &minR, &minG, &minB);
                YUVToRGB(nValue, 1, nSaturation, &maxR, &maxG, &maxB);
            } else { // Y
                YUVToRGB(0, nHue, nSaturation, &minR, &minG, &minB);
                YUVToRGB(1, nHue, nSaturation, &maxR, &maxG, &maxB);
            }

            // Clamp
            minR = qBound(0.0, minR, 1.0);
            minG = qBound(0.0, minG, 1.0);
            minB = qBound(0.0, minB, 1.0);

            maxR = qBound(0.0, maxR, 1.0);
            maxG = qBound(0.0, maxG, 1.0);
            maxB = qBound(0.0, maxB, 1.0);

            QColor minC, maxC;
            minC.setRgbF(minR, minG, minB);
            maxC.setRgbF(maxR, maxG, maxB);

            KoColor minK(minC, cs);
            KoColor maxK(maxC, cs);

            slider->setColors(minK, maxK);
            break;
        }
        }
    }

    void reset(KisHsvColorSlider *slider) const {
        slider->setValue(m_resetValue);
    }

    double scale(bool colorize, double value) const {
        if (colorize) {
            return value / m_max;
        }
        return value / m_maxRelative;
    }

    double normalize(bool colorize, double value) const {
        if (colorize) {
            return (value - m_min) / (m_max - m_min);
        }
        return (value - m_minRelative) / (m_maxRelative - m_minRelative);
    }

    SLIDER_TYPE m_type;
    KisHsvColorSlider::MIX_MODE m_mixMode;
    KLocalizedString m_title;
    int m_min, m_max;
    int m_minRelative, m_maxRelative;
    int m_resetValue;
};

// Slider configuration based on their SLIDER_TYPE
const SliderSettings SLIDER_TABLE[9] = {
    SliderSettings(SLIDER_TYPE::HUE,        KisHsvColorSlider::MIX_MODE::HSV,         ki18n("Hue"),         0,    360, -180, 180, 0),
    SliderSettings(SLIDER_TYPE::SATURATION, KisHsvColorSlider::MIX_MODE::HSV,         ki18n("Saturation"),  0,    100, -100, 100, 0),
    SliderSettings(SLIDER_TYPE::VALUE,      KisHsvColorSlider::MIX_MODE::HSV,         ki18nc("Brightness level of HSV model", "Value"),       -100, 100, -100, 100, 0),
    SliderSettings(SLIDER_TYPE::LIGHTNESS,  KisHsvColorSlider::MIX_MODE::HSL,         ki18n("Lightness"),   -100, 100, -100, 100, 0),
    SliderSettings(SLIDER_TYPE::LUMA,       KisHsvColorSlider::MIX_MODE::HSY,         ki18n("Luma"),        -100, 100, -100, 100, 0),
    SliderSettings(SLIDER_TYPE::INTENSITY,  KisHsvColorSlider::MIX_MODE::HSI,         ki18nc("Brightness in HSI color model", "Intensity"),   -100, 100, -100, 100, 0),
    SliderSettings(SLIDER_TYPE::YELLOW_BLUE,KisHsvColorSlider::MIX_MODE::COLOR_SPACE, ki18n("Yellow-Blue"), 0,    100, -100, 100, 0),
    SliderSettings(SLIDER_TYPE::GREEN_RED,  KisHsvColorSlider::MIX_MODE::COLOR_SPACE, ki18n("Green-Red"),   0,    100, -100, 100, 0),
    SliderSettings(SLIDER_TYPE::LUMA_YUV,   KisHsvColorSlider::MIX_MODE::COLOR_SPACE, ki18n("Luma"),        -100, 100, -100, 100, 0),
};

// Defines which sliders to display in each set.
// One for each SLIDER_SET.
const std::array<SLIDER_TYPE, 3> SLIDER_SETS[5] = {
    { SLIDER_TYPE::HUE,         SLIDER_TYPE::SATURATION, SLIDER_TYPE::VALUE     },
    { SLIDER_TYPE::HUE,         SLIDER_TYPE::SATURATION, SLIDER_TYPE::LIGHTNESS },
    { SLIDER_TYPE::HUE,         SLIDER_TYPE::SATURATION, SLIDER_TYPE::INTENSITY },
    { SLIDER_TYPE::HUE,         SLIDER_TYPE::SATURATION, SLIDER_TYPE::LUMA      },
    { SLIDER_TYPE::YELLOW_BLUE, SLIDER_TYPE::GREEN_RED,  SLIDER_TYPE::LUMA_YUV  },
};

SliderSettings sliderSetting(SLIDER_TYPE type) {
    return SLIDER_TABLE[static_cast<int>(type)];
}

}

KisHSVAdjustmentFilter::KisHSVAdjustmentFilter()
        : KisColorTransformationFilter(id(), FiltersCategoryAdjustId, i18n("&HSV Adjustment..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    setSupportsPainting(true);
}

KisConfigWidget *KisHSVAdjustmentFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisHSVConfigWidget(parent);
}

KoColorTransformation *KisHSVAdjustmentFilter::createTransformation(const KoColorSpace *cs, const KisFilterConfigurationSP config) const
{
    QHash<QString, QVariant> params;
    if (config) {
        int type = config->getInt("type", 1);
        bool colorize = config->getBool("colorize", false);
        bool compatibilityMode = config->getBool("compatibilityMode", true);

        const std::array<SLIDER_TYPE, 3> sliderSet = SLIDER_SETS[static_cast<int>(type)];

        params["h"] = sliderSetting(sliderSet[0]).scale(colorize, config->getInt("h", 0));
        params["s"] = sliderSetting(sliderSet[1]).scale(colorize, config->getInt("s", 0));
        params["v"] = sliderSetting(sliderSet[2]).scale(colorize, config->getInt("v", 0));

        params["type"] = type;
        params["colorize"] = colorize;
        params["lumaRed"] = cs->lumaCoefficients()[0];
        params["lumaGreen"] = cs->lumaCoefficients()[1];
        params["lumaBlue"] = cs->lumaCoefficients()[2];
        params["compatibilityMode"] = compatibilityMode;
    }
    return cs->createColorTransformation("hsv_adjustment", params);
}

KisFilterConfigurationSP KisHSVAdjustmentFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("h", 0);
    config->setProperty("s", 0);
    config->setProperty("v", 0);
    config->setProperty("type", 1);
    config->setProperty("colorize", false);
    config->setProperty("compatibilityMode", false);
    return config;
}

KisHSVConfigWidget::KisHSVConfigWidget(QWidget *parent, Qt::WindowFlags f)
    : KisConfigWidget(parent, f)
    , m_prevColorize(false)
{
    m_page = new Ui_WdgHSVAdjustment();
    m_page->setupUi(this);

    connect(m_page->cmbType, SIGNAL(activated(int)), this, SLOT(configureSliderLimitsAndLabels()));
    connect(m_page->chkColorize, SIGNAL(toggled(bool)), this, SLOT(configureSliderLimitsAndLabels()));
    connect(m_page->chkCompatibilityMode, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->reset, SIGNAL(clicked(bool)), this, SLOT(resetFilter()));

    // connect horizontal sliders
    connect(m_page->hSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->sSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->vSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page->hSpinBox, SIGNAL(valueChanged(int)), m_page->hSlider, SLOT(setValue(int)));
    connect(m_page->sSpinBox, SIGNAL(valueChanged(int)), m_page->sSlider, SLOT(setValue(int)));
    connect(m_page->vSpinBox, SIGNAL(valueChanged(int)), m_page->vSlider, SLOT(setValue(int)));

    connect(m_page->hSlider, SIGNAL(valueChanged(int)), m_page->hSpinBox, SLOT(setValue(int)));
    connect(m_page->sSlider, SIGNAL(valueChanged(int)), m_page->sSpinBox, SLOT(setValue(int)));
    connect(m_page->vSlider, SIGNAL(valueChanged(int)), m_page->vSpinBox, SLOT(setValue(int)));

    connect(m_page->hSlider, SIGNAL(valueChanged(int)), this, SLOT(recolorSliders()));
    connect(m_page->sSlider, SIGNAL(valueChanged(int)), this, SLOT(recolorSliders()));
    connect(m_page->vSlider, SIGNAL(valueChanged(int)), this, SLOT(recolorSliders()));
}

KisHSVConfigWidget::~KisHSVConfigWidget()
{
    delete m_page;
}

KisPropertiesConfigurationSP KisHSVConfigWidget::configuration() const
{
    KisColorTransformationConfigurationSP c = new KisColorTransformationConfiguration(KisHSVAdjustmentFilter::id().id(), 0, KisGlobalResourcesInterface::instance());
    c->setProperty("h", m_page->hSlider->value());
    c->setProperty("s", m_page->sSlider->value());
    c->setProperty("v", m_page->vSlider->value());
    c->setProperty("type", m_page->cmbType->currentIndex());
    c->setProperty("colorize", m_page->chkColorize->isChecked());
    c->setProperty("compatibilityMode", m_page->chkCompatibilityMode->isChecked());
    return c;
}

void KisHSVConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const int type = config->getInt("type", 1);
    m_page->cmbType->setCurrentIndex(type);
    m_page->chkColorize->setChecked(config->getBool("colorize", false));
    m_page->hSlider->setValue(config->getInt("h", 0));
    m_page->sSlider->setValue(config->getInt("s", 0));
    m_page->vSlider->setValue(config->getInt("v", 0));
    m_page->chkCompatibilityMode->setChecked(config->getInt("compatibilityMode", true));

    configureSliderLimitsAndLabels();
}

void KisHSVConfigWidget::configureSliderLimitsAndLabels()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_page->cmbType->currentIndex() >= 0);

    const std::array<SLIDER_TYPE, 3> sliderSet = SLIDER_SETS[m_page->cmbType->currentIndex()];
    const bool colorize = m_page->chkColorize->isChecked();

    // Prevent the sliders from recoloring when setValue is triggered in apply()
    // Since some values will be invalid while inside this function if colorize is changed
    KisSignalsBlocker blocker(m_page->hSlider, m_page->sSlider, m_page->vSlider);

    sliderSetting(sliderSet[0]).apply(m_page->hLabel, m_page->hSlider, m_page->hSpinBox, m_prevColorize, colorize);
    sliderSetting(sliderSet[1]).apply(m_page->sLabel, m_page->sSlider, m_page->sSpinBox, m_prevColorize, colorize);
    sliderSetting(sliderSet[2]).apply(m_page->vLabel, m_page->vSlider, m_page->vSpinBox, m_prevColorize, colorize);

    recolorSliders();

    const bool compat = !m_page->chkColorize->isChecked() && m_page->cmbType->currentIndex() <= 3;

    m_page->chkCompatibilityMode->setEnabled(compat);
    m_prevColorize = colorize;

    Q_EMIT sigConfigurationItemChanged();
}

void KisHSVConfigWidget::recolorSliders()
{
    const SLIDER_SET set = static_cast<SLIDER_SET>(m_page->cmbType->currentIndex());
    const std::array<SLIDER_TYPE, 3> sliderSet = SLIDER_SETS[m_page->cmbType->currentIndex()];
    const bool colorize = m_page->chkColorize->isChecked();

    const double nh = sliderSetting(sliderSet[0]).normalize(colorize, m_page->hSlider->value());
    const double ns = sliderSetting(sliderSet[1]).normalize(colorize, m_page->sSlider->value());
    const double nv = sliderSetting(sliderSet[2]).normalize(colorize, m_page->vSlider->value());

    sliderSetting(sliderSet[0]).recolor(m_page->hSlider, set, colorize, nh, ns, nv);
    sliderSetting(sliderSet[1]).recolor(m_page->sSlider, set, colorize, nh, ns, nv);
    sliderSetting(sliderSet[2]).recolor(m_page->vSlider, set, colorize, nh, ns, nv);
}

void KisHSVConfigWidget::resetFilter()
{
    const std::array<SLIDER_TYPE, 3> sliderSet = SLIDER_SETS[m_page->cmbType->currentIndex()];

    sliderSetting(sliderSet[0]).reset(m_page->hSlider);
    sliderSetting(sliderSet[1]).reset(m_page->sSlider);
    sliderSetting(sliderSet[2]).reset(m_page->vSlider);
}

