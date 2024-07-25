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
#include "kis_color_source_option.h"
#include "KisSharpnessOption.h"
#include "KisStandardOptions.h"
#include "KisDarkenOption.h"
#include "KisHSVOption.h"
#include "kis_texture_option.h"
#include "kis_painter.h"
#include "kis_paintop_settings.h"

struct KisBrushOpResources::Private
{
    Private(const KisPaintOpSettings *setting)
        : mixOption(setting)
        , darkenOption(setting)
    {
    }

    QList<KisHSVOption*> hsvOptions;
    KoColorTransformation *hsvTransformation = 0;
    KisMixOption mixOption;
    KisDarkenOption darkenOption;
};


KisBrushOpResources::KisBrushOpResources(const KisPaintOpSettingsSP settings, KisPainter *painter)
    : m_d(new Private(settings.data()))
{
    KisColorSourceOption colorSourceOption(settings.data());
    colorSource.reset(colorSourceOption.createColorSource(painter));

    sharpnessOption.reset(new KisSharpnessOption(settings.data()));

    textureOption.reset(new KisTextureOption(settings.data(),
                                             settings->resourcesInterface(),
                                             settings->canvasResourcesInterface(),
                                             painter->device()->defaultBounds()->currentLevelOfDetail(),
                                             SupportsGradientMode | SupportsLightnessMode));

    m_d->hsvOptions.append(KisHSVOption::createHueOption(settings.data()));
    m_d->hsvOptions.append(KisHSVOption::createSaturationOption(settings.data()));
    m_d->hsvOptions.append(KisHSVOption::createValueOption(settings.data()));

    Q_FOREACH (KisHSVOption *option, m_d->hsvOptions) {
        if (option->isChecked() && !m_d->hsvTransformation) {
            m_d->hsvTransformation = painter->backgroundColor().colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }
    }

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
        Q_FOREACH (KisHSVOption * option, m_d->hsvOptions) {
            option->apply(m_d->hsvTransformation, info);
        }
        colorSource->applyColorTransformation(m_d->hsvTransformation);
    }

    KisDabCacheUtils::DabRenderingResources::syncResourcesToSeqNo(seqNo, info);
}
