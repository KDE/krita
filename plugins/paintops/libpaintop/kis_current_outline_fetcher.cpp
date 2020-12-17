/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_current_outline_fetcher.h"

#include "kis_pressure_size_option.h"
#include "kis_pressure_rotation_option.h"
#include "kis_pressure_mirror_option.h"
#include "kis_pressure_sharpness_option.h"
#include <brushengine/kis_paintop_settings.h>
#include <kis_properties_configuration.h>
#include "kis_paintop_settings.h"
#include <QElapsedTimer>
#include "kis_algebra_2d.h"

#define NOISY_UPDATE_SPEED 50  // Time in ms for outline updates to noisy brushes

struct KisCurrentOutlineFetcher::Private {
    Private(Options optionsAvailable)
        : isDirty(true) {
        if (optionsAvailable & SIZE_OPTION) {
            sizeOption.reset(new KisPressureSizeOption());
        }

        if (optionsAvailable & ROTATION_OPTION) {
            rotationOption.reset(new KisPressureRotationOption());
        }

        if (optionsAvailable & MIRROR_OPTION) {
            mirrorOption.reset(new KisPressureMirrorOption());
        }

        if (optionsAvailable & SHARPNESS_OPTION) {
            sharpnessOption.reset(new KisPressureSharpnessOption());
        }
    }

    QScopedPointer<KisPressureSizeOption> sizeOption;
    QScopedPointer<KisPressureRotationOption> rotationOption;
    QScopedPointer<KisPressureMirrorOption> mirrorOption;
    QScopedPointer<KisPressureSharpnessOption> sharpnessOption;

    bool isDirty;
    QElapsedTimer lastUpdateTime;

    qreal lastRotationApplied;
    qreal lastSizeApplied;
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

QPainterPath KisCurrentOutlineFetcher::fetchOutline(const KisPaintInformation &info,
                                                    const KisPaintOpSettingsSP settings,
                                                    const QPainterPath &originalOutline,
                                                    const KisPaintOpSettings::OutlineMode &mode,
                                                    qreal alignForZoom,
                                                    qreal additionalScale,
                                                    qreal additionalRotation,
                                                    bool tilt, qreal tiltcenterx, qreal tiltcentery) const
{
    if (d->isDirty) {
        if (d->sizeOption) {
            d->sizeOption->readOptionSetting(settings);
            d->sizeOption->resetAllSensors();
        }

        if (d->rotationOption) {
            d->rotationOption->readOptionSetting(settings);
            d->rotationOption->resetAllSensors();
        }

        if (d->mirrorOption) {
            d->mirrorOption->readOptionSetting(settings);
            d->mirrorOption->resetAllSensors();
        }

        if (d->sharpnessOption) {
            d->sharpnessOption->readOptionSetting(settings);
            d->sharpnessOption->resetAllSensors();
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
    if (d->sharpnessOption) {
        qint32 x = 0, y = 0;
        qreal subPixelX = 0.0, subPixelY = 0.0;
        d->sharpnessOption->apply(info, pos - hotSpot, x, y, subPixelX, subPixelY);
        pos = QPointF(x + subPixelX, y + subPixelY) + hotSpot;
    }

    // align cursor position to widget pixel grid to avoid noise
    pos = KisAlgebra2D::alignForZoom(pos, alignForZoom);

    QTransform T1 = QTransform::fromTranslate(-hotSpot.x(), -hotSpot.y());
    QTransform T2 = QTransform::fromTranslate(pos.x(), pos.y());
    QTransform S  = QTransform::fromScale(xFlip * scale, yFlip * scale);

    return (T1 * rot * S * T2).map(originalOutline);
}
