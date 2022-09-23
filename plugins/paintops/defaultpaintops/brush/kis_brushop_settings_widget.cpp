/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brushop_settings_widget.h"
#include <KisBrushOpSettings.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_flow_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_ratio_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_mix_option.h>
#include <kis_pressure_lightness_strength_option.h>
#include <kis_pressure_lightness_strength_option_widget.h>
#include <kis_pressure_hsv_option.h>
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_softness_option.h>
#include <kis_pressure_sharpness_option_widget.h>
#include <kis_color_source_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_flow_opacity_option_widget.h>
#include <kis_pressure_spacing_option_widget.h>
#include <kis_pressure_rate_option.h>
#include "kis_texture_option.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"
#include <KisMaskingBrushOption.h>

#include <KisPrefixedPaintOpOptionWrapper.h>
#include <KisPaintopSettingsIds.h>
#include "kis_brush_option_widget.h"
#include "KisCurveOptionWidget2.h"

#include <KisCurveOptionData.h>
#include <lager/state.hpp>
#include <KisZug.h>

#include <kis_paintop_lod_limitations.h>

class KisSizeOptionData : public KisCurveOptionData
{
public:
    KisSizeOptionData(bool isCheckable = false, const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Size", i18n("Size")),
              isCheckable,
              !isCheckable)
    {
        this->prefix = prefix;
    }

    KisPaintopLodLimitations lodLimitations() const
    {
        KisPaintopLodLimitations l;

        // HINT: FUZZY_PER_STROKE doesn't affect instant preview
        if (sensorFuzzyPerDab.isActive) {
            l.limitations << KoID("size-fade", i18nc("PaintOp instant preview limitation", "Size -> Fuzzy (sensor)"));
        }

        if (sensorFade.isActive) {
            l.blockers << KoID("size-fuzzy", i18nc("PaintOp instant preview limitation", "Size -> Fade (sensor)"));
        }

        return l;
    }
};

class KisOpacityOptionData : public KisCurveOptionData
{
public:
    KisOpacityOptionData()
        : KisCurveOptionData(
              KoID("Opacity", i18n("Opacity")),
              false, true)
    {}
};

class KisFlowOptionData : public KisCurveOptionData
{
public:
    KisFlowOptionData(bool isCheckable = false, const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Flow", i18n("Flow")),
              isCheckable,
              !isCheckable)
    {
        this->prefix = prefix;
    }
};

class KisRatioOptionData : public KisCurveOptionData
{
public:
    KisRatioOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Ratio", i18n("Ratio")))
    {
        this->prefix = prefix;
    }
};

class KisSoftnessOptionData : public KisCurveOptionData
{
public:
    KisSoftnessOptionData()
        : KisCurveOptionData(
              KoID("Softness", i18n("Softness")),
              true, false, false,
              0.1, 1.0)
    {}
};

class KisRotationOptionData : public KisCurveOptionData
{
public:
    KisRotationOptionData(const QString &prefix = QString())
        : KisCurveOptionData(
              KoID("Rotation", i18n("Rotation")))
    {
        this->prefix = prefix;
    }
};

class KisDarkenOptionData : public KisCurveOptionData
{
public:
    KisDarkenOptionData()
        : KisCurveOptionData(
              KoID("Darken", i18n("Darken")))
    {}
};

class KisMixOptionData : public KisCurveOptionData
{
public:
    KisMixOptionData()
        : KisCurveOptionData(
              KoID("Mix", i18nc("Mixing of colors", "Mix")))
    {}
};

class KisHueOptionData : public KisCurveOptionData
{
public:
    KisHueOptionData()
        : KisCurveOptionData(
              KoID("h", i18n("Hue")))
    {}
};

class KisSaturationOptionData : public KisCurveOptionData
{
public:
    KisSaturationOptionData()
        : KisCurveOptionData(
              KoID("s", i18n("Saturation")))
    {}
};

class KisValueOptionData : public KisCurveOptionData
{
public:
    KisValueOptionData()
        : KisCurveOptionData(
              KoID("v", i18nc("Label of Brightness value in Color Smudge brush engine options", "Value")))
    {}
};

class KisRateOptionData : public KisCurveOptionData
{
public:
    KisRateOptionData()
        : KisCurveOptionData(
              KoID("Rate", i18n("Rate")))
    {}
};

class KisStrengthOptionData : public KisCurveOptionData
{
public:
    KisStrengthOptionData()
        : KisCurveOptionData(
              KoID("Texture/Strength/", i18n("Strength")))
    {}
};


struct KisBrushOpSettingsWidget::Private
{
    Private()
        : maskingSizeOptionData({true, KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingFlowOptionData({true, KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingRatioOptionData({KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingRotationOptionData({KisPaintOpUtils::MaskingBrushPresetPrefix}),
          lodLimitations(sizeOptionData.xform(zug::map(&KisSizeOptionData::lodLimitations)))
    {
    }

    lager::state<KisOpacityOptionData, lager::automatic_tag> opacityOptionData;
    lager::state<KisSizeOptionData, lager::automatic_tag> sizeOptionData;
    lager::state<KisFlowOptionData, lager::automatic_tag> flowOptionData;
    lager::state<KisRatioOptionData, lager::automatic_tag> ratioOptionData;
    lager::state<KisSoftnessOptionData, lager::automatic_tag> softnessOptionData;
    lager::state<KisRotationOptionData, lager::automatic_tag> rotationOptionData;

    lager::state<KisDarkenOptionData, lager::automatic_tag> darkenOptionData;
    lager::state<KisMixOptionData, lager::automatic_tag> mixOptionData;
    lager::state<KisHueOptionData, lager::automatic_tag> hueOptionData;
    lager::state<KisSaturationOptionData, lager::automatic_tag> saturationOptionData;
    lager::state<KisValueOptionData, lager::automatic_tag> valueOptionData;
    lager::state<KisRateOptionData, lager::automatic_tag> rateOptionData;

    lager::state<KisStrengthOptionData, lager::automatic_tag> strengthOptionData;

    lager::state<KisSizeOptionData, lager::automatic_tag> maskingSizeOptionData;
    lager::state<KisFlowOptionData, lager::automatic_tag> maskingFlowOptionData;
    lager::state<KisRatioOptionData, lager::automatic_tag> maskingRatioOptionData;
    lager::state<KisRotationOptionData, lager::automatic_tag> maskingRotationOptionData;

    lager::reader<KisPaintopLodLimitations> lodLimitations;
};

KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision |
                                       KisBrushOptionWidgetFlag::SupportsHSLBrushMode,
                                       parent),
      m_d(new Private)
{
    setObjectName("brush option widget");

    // Brush tip options
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOptionData(m_d->opacityOptionData, KisPaintOpOption::GENERAL, i18n("Transparent"), i18n("Opaque"));
    addPaintOpOptionData(m_d->flowOptionData, KisPaintOpOption::GENERAL);
    addPaintOpOptionData(m_d->sizeOptionData, KisPaintOpOption::GENERAL);
    addPaintOpOptionData(m_d->ratioOptionData, KisPaintOpOption::GENERAL);
    addPaintOpOption(new KisPressureSpacingOptionWidget());
    addPaintOpOption(new KisPressureMirrorOptionWidget());

    addPaintOpOptionData(m_d->softnessOptionData, KisPaintOpOption::GENERAL, i18n("Soft"), i18n("Hard"));
    addPaintOpOptionData(m_d->rotationOptionData, KisPaintOpOption::GENERAL, i18n("-180°"), i18n("180°"));
    addPaintOpOption(new KisPressureSharpnessOptionWidget());
    m_lightnessStrengthOptionWidget = new KisPressureLightnessStrengthOptionWidget();
    addPaintOpOption(m_lightnessStrengthOptionWidget);
    addPaintOpOption(new KisPressureScatterOptionWidget());

    // Colors options
    addPaintOpOption(new KisColorSourceOptionWidget());
    addPaintOpOptionData(m_d->darkenOptionData, KisPaintOpOption::COLOR, i18n("0.0"), i18n("1.0"));
    addPaintOpOptionData(m_d->mixOptionData, KisPaintOpOption::COLOR, i18nc("Background painting color", "Background"), i18n("Foreground painting color", "Foreground"));

    addPaintOpOptionData(m_d->hueOptionData,
                         KisPaintOpOption::COLOR,
                         KisPressureHSVOption::hueMinLabel(),
                         KisPressureHSVOption::hueMaxLabel(),
                         -180, 180, i18n("°"));

    addPaintOpOptionData(m_d->saturationOptionData,
                         KisPaintOpOption::COLOR,
                         KisPressureHSVOption::saturationMinLabel(),
                         KisPressureHSVOption::saturationMaxLabel());

    addPaintOpOptionData(m_d->valueOptionData,
                         KisPaintOpOption::COLOR,
                         KisPressureHSVOption::valueMinLabel(),
                         KisPressureHSVOption::valueMaxLabel());

    addPaintOpOption(new KisAirbrushOptionWidget(false));
    addPaintOpOptionData(m_d->rateOptionData, KisPaintOpOption::COLOR);

    KisPaintActionTypeOption *actionTypeOption = new KisPaintActionTypeOption();
    addPaintOpOption(actionTypeOption);

    addPaintOpOption(new KisTextureOption(SupportsLightnessMode | SupportsGradientMode));
    addPaintOpOptionData(m_d->strengthOptionData, KisPaintOpOption::COLOR, i18n("Weak"), i18n("Strong"));

    KisMaskingBrushOption *maskingOption = new KisMaskingBrushOption(brushOptionWidget()->effectiveBrushSize());
    addPaintOpOption(maskingOption);

    connect(maskingOption, SIGNAL(sigCheckedChanged(bool)),
            actionTypeOption, SLOT(slotForceWashMode(bool)));

    addPaintOpOptionData(m_d->maskingSizeOptionData, KisPaintOpOption::MASKING_BRUSH);
    addPaintOpOptionData(m_d->maskingFlowOptionData, KisPaintOpOption::MASKING_BRUSH);
    addPaintOpOptionData(m_d->maskingRatioOptionData, KisPaintOpOption::MASKING_BRUSH);
    addPaintOpOptionData(m_d->maskingRotationOptionData, KisPaintOpOption::MASKING_BRUSH, i18n("-180°"), i18n("180°"));

    addPaintOpOption(new KisPrefixedPaintOpOptionWrapper<KisFlowOpacityOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix),
                     KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(new KisPrefixedPaintOpOptionWrapper<KisPressureMirrorOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix),
                     KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(new KisPrefixedPaintOpOptionWrapper<KisPressureScatterOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix),
                     KisPaintOpOption::MASKING_BRUSH);
}

KisBrushOpSettingsWidget::~KisBrushOpSettingsWidget()
{
}

KisPropertiesConfigurationSP KisBrushOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettingsSP config = new KisBrushOpSettings(resourcesInterface());
    config->setProperty("paintop", "paintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisBrushOpSettingsWidget::notifyPageChanged()
{
    m_lightnessStrengthOptionWidget->setEnabled(this->brushOptionWidget()->preserveLightness());
}

template<typename Data, typename... Args>
void KisBrushOpSettingsWidget::addPaintOpOptionData(Data &data, Args... args)
{
    addPaintOpOption(new KisCurveOptionWidget2(data.zoom(kiszug::lenses::do_static_cast<const typename Data::value_type&, const KisCurveOptionData&>),
                                                         args...));
}
