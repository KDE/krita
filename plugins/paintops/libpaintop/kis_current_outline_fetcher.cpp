/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_current_outline_fetcher.h"

#include "kis_pressure_size_option.h"
#include "kis_pressure_rotation_option.h"
#include "kis_pressure_mirror_option.h"
#include <brushengine/kis_paintop_settings.h>
#include <kis_properties_configuration.h>
#include "kis_paintop_settings.h"
#include <QElapsedTimer>

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
    }

    QScopedPointer<KisPressureSizeOption> sizeOption;
    QScopedPointer<KisPressureRotationOption> rotationOption;
    QScopedPointer<KisPressureMirrorOption> mirrorOption;

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
        rotation += settings->getDouble("runtimeCanvasRotation", 0.0) * M_PI / 180.0;
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
    QTransform T1 = QTransform::fromTranslate(-hotSpot.x(), -hotSpot.y());
    QTransform T2 = QTransform::fromTranslate(info.pos().x(), info.pos().y());
    QTransform S  = QTransform::fromScale(xFlip * scale, yFlip * scale);

    return (T1 * rot * S * T2).map(originalOutline);
}
