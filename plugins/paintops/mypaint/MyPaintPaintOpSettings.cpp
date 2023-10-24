/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintPaintOpSettings.h"

#include <cmath>
#include <KisOptimizedBrushOutline.h>
#include <MyPaintStandardOptionData.h>

struct KisMyPaintOpSettings::Private
{
    bool cachedProperties{false};
    qreal paintOpSize{0.0};
    qreal paintOpAngle{0.0};  // the original angle value as seen by KisPaintOpSettings interface
    qreal offsetValue{0.0};
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
    return 2 * exp(m_d->paintOpSize);
}

void KisMyPaintOpSettings::setPaintOpAngle(qreal value)
{
    MyPaintEllipticalDabAngleData data;
    data.read(this);

    // Paintop angle is expressed in degrees.
    value = normalizeAngleDegrees(value);

    // To keep the angle consistent across the KisPaintOpSettings interface, we keep track of the original value.
    const qreal angleDifference = value - m_d->paintOpAngle;
    m_d->paintOpAngle = value;

    // We need to reverse the rotation (MyPaint is clockwise). To do this, we subtract the angle difference
    // instead of adding it to the old value.
    value = data.strengthValue - angleDifference;

    // Shift the angle to zero, so that we can use fmod to wrap around
    value -= data.strengthMinValue;

    // Wrap the angle in [0, range]
    const qreal range = data.strengthMaxValue - data.strengthMinValue;
    if (value < 0)
    {
        value = range + std::fmod(value, range);
    }
    else if (value > range)
    {
        value = std::fmod(value, range);
    }

    // Shift back to the correct min/max range
    value += data.strengthMinValue;

    data.strengthValue = value;
    data.write(this);
}

qreal KisMyPaintOpSettings::paintOpAngle() const
{
    return m_d->paintOpAngle;
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

void KisMyPaintOpSettings::resetSettings(const QStringList &preserveProperties)
{
    QStringList allKeys = preserveProperties;
    allKeys << MYPAINT_JSON;
    KisOutlineGenerationPolicy<KisPaintOpSettings>::resetSettings(allKeys);
}

void KisMyPaintOpSettings::onPropertyChanged()
{
    m_d->cachedProperties = false;
    KisOutlineGenerationPolicy::onPropertyChanged();
}

KisOptimizedBrushOutline KisMyPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    KisOptimizedBrushOutline path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;
        if (!m_d->cachedProperties) {
            {
                MyPaintOffsetByRandomData data;
                data.read(this);
                m_d->offsetValue = data.strengthValue;
            }
            {
                MyPaintRadiusLogarithmicData data;
                data.read(this);
                m_d->paintOpSize = data.strengthValue;
            }
            m_d->cachedProperties = true;
        }

        const qreal offset = m_d->offsetValue;

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
