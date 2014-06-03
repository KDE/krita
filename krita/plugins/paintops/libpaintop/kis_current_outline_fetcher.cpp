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
#include "kis_paintop_settings.h"


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
};

KisCurrentOutlineFetcher::KisCurrentOutlineFetcher(Options optionsAvailable)
    : d(new Private(optionsAvailable))
{
}

KisCurrentOutlineFetcher::~KisCurrentOutlineFetcher()
{
}

void KisCurrentOutlineFetcher::setDirty()
{
    d->isDirty = true;
}

QPainterPath KisCurrentOutlineFetcher::fetchOutline(const KisPaintInformation &info,
                                                    const KisPaintOpSettings *settings,
                                                    const QPainterPath &originalOutline,
                                                    qreal additionalScale,
                                                    qreal additionalRotation) const
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
    MirrorProperties mirrorProperties;

    if (d->sizeOption) {
        scale *= d->sizeOption->apply(info);
    }

    if (d->rotationOption) {
        rotation += d->rotationOption->apply(info);
    }

    qreal xFlip = 1.0;
    qreal yFlip = 1.0;

    if (d->mirrorOption) {
        mirrorProperties = d->mirrorOption->apply(info);

        if (mirrorProperties.coordinateSystemFlipped) {
            rotation = 2 * M_PI - rotation;
        }

        if (mirrorProperties.horizontalMirror) {
            xFlip = -1.0;
        }

        if (mirrorProperties.verticalMirror) {
            yFlip = -1.0;
        }
    }

    QPointF hotSpot = originalOutline.boundingRect().center();

    QTransform T1 = QTransform::fromTranslate(-hotSpot.x(), -hotSpot.y());
    QTransform S = QTransform::fromScale(xFlip * scale, yFlip * scale);

    QTransform rot;
    rot.rotateRadians(-rotation);

    QTransform T2 = QTransform::fromTranslate(info.pos().x(), info.pos().y());

    return (T1 * rot * S * T2).map(originalOutline);
}
