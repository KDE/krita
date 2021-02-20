/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_liquify_paintop.h"

#include <QPainterPath>
#include <QTransform>


#include <brushengine/kis_paint_information.h>
#include "kis_liquify_transform_worker.h"
#include "kis_algebra_2d.h"
#include "kis_liquify_properties.h"
#include "kis_spacing_information.h"
#include "kis_timing_information.h"


struct KisLiquifyPaintop::Private
{
    Private(const KisLiquifyProperties &_props, KisLiquifyTransformWorker *_worker)
        : props(_props), worker(_worker) {}

    KisLiquifyProperties props;
    KisLiquifyTransformWorker *worker;
};

KisLiquifyPaintop::KisLiquifyPaintop(const KisLiquifyProperties &props, KisLiquifyTransformWorker *worker)
    : m_d(new Private(props, worker))
{
}

KisLiquifyPaintop::~KisLiquifyPaintop()
{
}

QPainterPath KisLiquifyPaintop::brushOutline(const KisLiquifyProperties &props,
                                             const KisPaintInformation &info)
{
    const qreal diameter = props.size();
    const qreal reverseCoeff = props.reverseDirection() ? -1.0 : 1.0;

    QPainterPath outline;
    outline.addEllipse(-0.5 * diameter, -0.5 * diameter,
                       diameter, diameter);

    switch (props.mode()) {
    case KisLiquifyProperties::MOVE:
    case KisLiquifyProperties::SCALE:
        break;
    case KisLiquifyProperties::ROTATE: {
        QPainterPath p;
        p.lineTo(-3.0, 4.0);
        p.moveTo(0.0, 0.0);
        p.lineTo(-3.0, -4.0);

        QTransform S;
        if (diameter < 15.0) {
            const qreal scale = diameter / 15.0;
            S = QTransform::fromScale(scale, scale);
        }
        QTransform R;
        R.rotateRadians(-reverseCoeff * 0.5 * M_PI);
        QTransform T = QTransform::fromTranslate(0.5 * diameter, 0.0);

        p = (S * R * T).map(p);
        outline.addPath(p);

        break;
    }
    case KisLiquifyProperties::OFFSET: {
        qreal normalAngle = info.drawingAngle() + reverseCoeff * 0.5 * M_PI;

        QPainterPath p = KisAlgebra2D::smallArrow();

        const qreal offset = qMax(0.8 * diameter, 15.0);

        QTransform R;
        R.rotateRadians(normalAngle);
        QTransform T = QTransform::fromTranslate(offset, 0.0);
        p = (T * R).map(p);

        outline.addPath(p);

        break;
    }
    case KisLiquifyProperties::UNDO:
        break;
    case KisLiquifyProperties::N_MODES:
        qFatal("Not supported mode");
    }

    return outline;
}

// TODO: Reduce code duplication between KisLiquifyPaintop and KisPaintOp. It might be possible to
// make them both subclasses of some more general base class.
void KisLiquifyPaintop::updateSpacing(const KisPaintInformation &info,
                               KisDistanceInformation &currentDistance) const
{
    KisPaintInformation pi(info);
    KisSpacingInformation spacingInfo;
    {
        KisPaintInformation::DistanceInformationRegistrar r
            = pi.registerDistanceInformation(&currentDistance);
        spacingInfo = updateSpacingImpl(pi);
    }

    currentDistance.updateSpacing(spacingInfo);
}

void KisLiquifyPaintop::updateTiming(const KisPaintInformation &info,
                                     KisDistanceInformation &currentDistance) const
{
    KisPaintInformation pi(info);
    KisTimingInformation timingInfo;
    {
        KisPaintInformation::DistanceInformationRegistrar r
            = pi.registerDistanceInformation(&currentDistance);
        timingInfo = updateTimingImpl(pi);
    }

    currentDistance.updateTiming(timingInfo);
}

KisSpacingInformation KisLiquifyPaintop::paintAt(const KisPaintInformation &pi)
{
    const qreal size = computeSize(pi);

    const qreal spacing = m_d->props.spacing() * size;

    const qreal reverseCoeff =
        m_d->props.mode() !=
        KisLiquifyProperties::UNDO &&
        m_d->props.reverseDirection() ? -1.0 : 1.0;
    const qreal amount = m_d->props.amountHasPressure() ?
        pi.pressure() * reverseCoeff * m_d->props.amount():
        reverseCoeff * m_d->props.amount();

    const bool useWashMode = m_d->props.useWashMode();
    const qreal flow = m_d->props.flow();

    switch (m_d->props.mode()) {
    case KisLiquifyProperties::MOVE: {
        const qreal offsetLength = size * amount;
        m_d->worker->translatePoints(pi.pos(),
                                     pi.drawingDirectionVector() * offsetLength,
                                     size, useWashMode, flow);

        break;
    }
    case KisLiquifyProperties::SCALE:
        m_d->worker->scalePoints(pi.pos(),
                                 amount,
                                 size, useWashMode, flow);
        break;
    case KisLiquifyProperties::ROTATE:
        m_d->worker->rotatePoints(pi.pos(),
                                  2.0 * M_PI * amount,
                                  size, useWashMode, flow);
        break;
    case KisLiquifyProperties::OFFSET: {
        const qreal offsetLength = size * amount;
        m_d->worker->translatePoints(pi.pos(),
                                     KisAlgebra2D::rightUnitNormal(pi.drawingDirectionVector()) * offsetLength,
                                     size, useWashMode, flow);
        break;
    }
    case KisLiquifyProperties::UNDO:
        m_d->worker->undoPoints(pi.pos(),
                                amount,
                                size);

        break;
    case KisLiquifyProperties::N_MODES:
        qFatal("Not supported mode");
    }

    return KisSpacingInformation(spacing);
}

KisSpacingInformation KisLiquifyPaintop::updateSpacingImpl(const KisPaintInformation &pi) const
{
    return KisSpacingInformation(m_d->props.spacing() * computeSize(pi));
}

KisTimingInformation KisLiquifyPaintop::updateTimingImpl(const KisPaintInformation &pi) const
{
    Q_UNUSED(pi);
    // Don't use airbrushing.
    return KisTimingInformation();
}

qreal KisLiquifyPaintop::computeSize(const KisPaintInformation &pi) const
{
    static const qreal sizeToSigmaCoeff = 1.0 / 3.0;
    return sizeToSigmaCoeff *
        (m_d->props.sizeHasPressure() ?
         pi.pressure() * m_d->props.size():
         m_d->props.size());
}
