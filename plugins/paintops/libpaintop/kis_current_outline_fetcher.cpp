/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_current_outline_fetcher.h"

#include <KisOpacityOption.h>
#include <KisRotationOption.h>
#include <KisMirrorOption.h>
#include <KisSharpnessOption.h>
#include <KisMirrorProperties.h>
#include <brushengine/kis_paintop_settings.h>
#include <kis_properties_configuration.h>
#include "kis_paintop_settings.h"
#include <QElapsedTimer>
#include "kis_algebra_2d.h"
#include "KisOptimizedBrushOutline.h"

#define NOISY_UPDATE_SPEED 50  // Time in ms for outline updates to noisy brushes

struct KisCurrentOutlineFetcher::Private {
    Private(Options _optionsAvailable)
        : optionsAvailable(_optionsAvailable)
        , isDirty(true)
    {
    }

    Options optionsAvailable;

    QScopedPointer<KisSizeOption> sizeOption;
    QScopedPointer<KisRotationOption> rotationOption;
    QScopedPointer<KisMirrorOption> mirrorOption;
    QScopedPointer<KisSharpnessOption> sharpnessOption;

    bool isDirty {false};
    QElapsedTimer lastUpdateTime;

    qreal lastRotationApplied {0.0};
    qreal lastSizeApplied {1.0};
    MirrorProperties lastMirrorApplied;
};

KisCurrentOutlineFetcher::KisCurrentOutlineFetcher(Options optionsAvailable)
    : d(new Private(optionsAvailable))
{
    d->lastUpdateTime.start();
}

KisCurrentOutlineFetcher::~KisCurrentOutlineFetcher()
{
}

void KisCurrentOutlineFetcher::setDirty()
{
    d->isDirty = true;
}

KisOptimizedBrushOutline KisCurrentOutlineFetcher::fetchOutline(const KisPaintInformation &info, const KisPaintOpSettingsSP settings, const KisOptimizedBrushOutline &originalOutline, const KisPaintOpSettings::OutlineMode &mode, qreal alignForZoom, qreal additionalScale, qreal additionalRotation, bool tilt, qreal tiltcenterx, qreal tiltcentery) const
{
    if (d->isDirty) {
        if (d->optionsAvailable & SIZE_OPTION) {
            d->sizeOption.reset(new KisSizeOption(settings.data()));
        }

        if (d->optionsAvailable & ROTATION_OPTION) {
            d->rotationOption.reset(new KisRotationOption(settings.data()));
        }

        if (d->optionsAvailable & MIRROR_OPTION) {
            d->mirrorOption.reset(new KisMirrorOption(settings.data()));
        }

        if (d->optionsAvailable & SHARPNESS_OPTION) {
            d->sharpnessOption.reset(new KisSharpnessOption(settings.data()));
        }

        d->isDirty = false;
    }

    qreal scale = additionalScale;
    qreal rotation = additionalRotation;
    bool needsUpdate = false;

    // Randomized rotation at full speed looks noisy, so slow it down
    if (d->lastUpdateTime.elapsed() > NOISY_UPDATE_SPEED) {
        needsUpdate = true;
        d->lastUpdateTime.restart();
    }

    if (d->sizeOption && !tilt && !mode.forceFullSize) {
        if (!d->sizeOption->isRandom() || needsUpdate) {
            d->lastSizeApplied = d->sizeOption->apply(info);
        }
        scale *= d->lastSizeApplied;
    }

    if (d->rotationOption && !tilt) {
        if (!d->rotationOption->isRandom() || needsUpdate) {
            d->lastRotationApplied = d->rotationOption->apply(info);
        }
        rotation += d->lastRotationApplied;
    } else if (d->rotationOption && tilt) {
        rotation += info.canvasRotation() * M_PI / 180.0;
    }

    qreal xFlip = 1.0;
    qreal yFlip = 1.0;

    if (d->mirrorOption) {
        if (!d->mirrorOption->isRandom() || needsUpdate) {
            d->lastMirrorApplied = d->mirrorOption->apply(info);
        }

        if (d->lastMirrorApplied.coordinateSystemFlipped) {
            rotation = 2 * M_PI - rotation;
        }

        if (d->lastMirrorApplied.horizontalMirror) {
            xFlip = -1.0;
        }

        if (d->lastMirrorApplied.verticalMirror) {
            yFlip = -1.0;
        }
    }

    QTransform rot;
    rot.rotateRadians(-rotation);

    QPointF hotSpot = originalOutline.boundingRect().center();
    if (tilt) {
        hotSpot.setX(tiltcenterx);
        hotSpot.setY(tiltcentery);
    }

    QPointF pos = info.pos();
    if (d->sharpnessOption && d->sharpnessOption->alignOutlineToPixels()) {
        qint32 x = 0;
        qint32 y = 0;
        qreal subPixelX = 0.0;
        qreal subPixelY = 0.0;
        d->sharpnessOption->apply(info, pos - hotSpot, x, y, subPixelX, subPixelY);
        pos = QPointF(x + subPixelX, y + subPixelY) + hotSpot;
    }

    // align cursor position to widget pixel grid to avoid noise
    pos = KisAlgebra2D::alignForZoom(pos, alignForZoom);

    QTransform T1 = QTransform::fromTranslate(-hotSpot.x(), -hotSpot.y());
    QTransform T2 = QTransform::fromTranslate(pos.x(), pos.y());
    QTransform S  = QTransform::fromScale(xFlip * scale, yFlip * scale);

    return originalOutline.mapped(T1 * rot * S * T2);
}
