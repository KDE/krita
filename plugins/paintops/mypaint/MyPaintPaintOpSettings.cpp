/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintPaintOpSettings.h"

#include <cmath>
#include <kis_color_option.h>
#include <KisOptimizedBrushOutline.h>
#include <MyPaintStandardOptionData.h>

struct KisMyPaintOpSettings::Private
{
};


KisMyPaintOpSettings::KisMyPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION,
                                                     resourcesInterface),
    m_d(new Private)
{
}

KisMyPaintOpSettings::~KisMyPaintOpSettings()
{
}

void KisMyPaintOpSettings::setPaintOpSize(qreal value)
{
    MyPaintRadiusLogarithmicData data;
    data.read(this);
    data.strengthValue = log(0.5 * value);
    data.write(this);
}

qreal KisMyPaintOpSettings::paintOpSize() const
{
    MyPaintRadiusLogarithmicData data;
    data.read(this);
    return 2 * exp(data.strengthValue);
}

void KisMyPaintOpSettings::setPaintOpOpacity(qreal value)
{
    MyPaintOpacityData data;
    data.read(this);
    data.strengthValue = value;
    data.write(this);
}

qreal KisMyPaintOpSettings::paintOpOpacity()
{
    MyPaintOpacityData data;
    data.read(this);
    return data.strengthValue;
}

bool KisMyPaintOpSettings::paintIncremental()
{
    return true;
}

KisOptimizedBrushOutline KisMyPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    KisOptimizedBrushOutline path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;

        MyPaintOffsetByRandomData data;
        data.read(this);
        const qreal offset = data.strengthValue;

        qreal radius = 0.5 * paintOpSize();
        radius = radius + 2 * radius * offset;
        radius = qBound(3.5, radius, 500.0);

        QPainterPath realOutline;
        realOutline.addEllipse(QPointF(), radius, radius);

        path = outlineFetcher()->fetchOutline(info, this, realOutline, mode, alignForZoom, finalScale);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info,
                realOutline.boundingRect().center(),
                realOutline.boundingRect().width() * 0.5,
                3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, finalScale, 0.0, true, realOutline.boundingRect().center().x(), realOutline.boundingRect().center().y()));
        }
    }

    return path;
}
