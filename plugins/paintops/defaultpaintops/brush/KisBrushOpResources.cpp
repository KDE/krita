/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushOpResources.h"

#include "kis_brush.h"

#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include "kis_color_source.h"
#include "kis_pressure_mix_option.h"
#include "kis_pressure_darken_option.h"
#include "kis_pressure_hsv_option.h"
#include "kis_color_source_option.h"
#include "kis_pressure_sharpness_option.h"
#include "kis_texture_option.h"
#include "kis_painter.h"
#include "kis_paintop_settings.h"

struct KisBrushOpResources::Private
{
    QList<KisPressureHSVOption*> hsvOptions;
    KoColorTransformation *hsvTransformation = 0;
    KisPressureMixOption mixOption;
    KisPressureDarkenOption darkenOption;
};


KisBrushOpResources::KisBrushOpResources(const KisPaintOpSettingsSP settings, KisPainter *painter)
    : m_d(new Private)
{
    KisColorSourceOption colorSourceOption;
    colorSourceOption.readOptionSetting(settings);
    colorSource.reset(colorSourceOption.createColorSource(painter));

    sharpnessOption.reset(new KisPressureSharpnessOption());
    sharpnessOption->readOptionSetting(settings);
    sharpnessOption->resetAllSensors();

    textureOption.reset(new KisTextureProperties(painter->device()->defaultBounds()->currentLevelOfDetail(), SupportsGradientMode | SupportsLightnessMode));
    textureOption->fillProperties(settings, settings->resourcesInterface(), settings->canvasResourcesInterface());

    m_d->hsvOptions.append(KisPressureHSVOption::createHueOption());
    m_d->hsvOptions.append(KisPressureHSVOption::createSaturationOption());
    m_d->hsvOptions.append(KisPressureHSVOption::createValueOption());

    Q_FOREACH (KisPressureHSVOption * option, m_d->hsvOptions) {
        option->readOptionSetting(settings);
        option->resetAllSensors();
        if (option->isChecked() && !m_d->hsvTransformation) {
            m_d->hsvTransformation = painter->backgroundColor().colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }
    }

    m_d->darkenOption.readOptionSetting(settings);
    m_d->mixOption.readOptionSetting(settings);

    m_d->darkenOption.resetAllSensors();
    m_d->mixOption.resetAllSensors();

    // the brush should be initialized explicitly by the caller later
    KIS_SAFE_ASSERT_RECOVER_NOOP(!brush);
}

KisBrushOpResources::~KisBrushOpResources()
{
    qDeleteAll(m_d->hsvOptions);
    delete m_d->hsvTransformation;
}

void KisBrushOpResources::syncResourcesToSeqNo(int seqNo, const KisPaintInformation &info)
{
    colorSource->selectColor(m_d->mixOption.apply(info), info);
    m_d->darkenOption.apply(colorSource.data(), info);

    if (m_d->hsvTransformation) {
        Q_FOREACH (KisPressureHSVOption * option, m_d->hsvOptions) {
            option->apply(m_d->hsvTransformation, info);
        }
        colorSource->applyColorTransformation(m_d->hsvTransformation);
    }

    KisDabCacheUtils::DabRenderingResources::syncResourcesToSeqNo(seqNo, info);
}
