/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_liquify_paintop.h"

#include "kis_paint_information.h"
#include "kis_liquify_transform_worker.h"
#include "kis_algebra_2d.h"


struct KisLiquifyPaintop::Private
{
    Private(const ToolTransformArgs::LiquifyProperties &_props, KisLiquifyTransformWorker *_worker)
        : props(_props), worker(_worker) {}

    ToolTransformArgs::LiquifyProperties props;
    KisLiquifyTransformWorker *worker;
};

KisLiquifyPaintop::KisLiquifyPaintop(const ToolTransformArgs::LiquifyProperties &props, KisLiquifyTransformWorker *worker)
    : m_d(new Private(props, worker))
{
}

KisLiquifyPaintop::~KisLiquifyPaintop()
{
}

QPainterPath KisLiquifyPaintop::brushOutline(const ToolTransformArgs::LiquifyProperties &props,
                                             const KisPaintInformation &info)
{
    const qreal diameter = props.size();
    const qreal reverseCoeff = props.reverseDirection() ? -1.0 : 1.0;

    QPainterPath outline;
    outline.addEllipse(-0.5 * diameter, -0.5 * diameter,
                       diameter, diameter);

    switch (props.currentMode()) {
    case ToolTransformArgs::LiquifyProperties::MOVE:
    case ToolTransformArgs::LiquifyProperties::SCALE:
        break;
    case ToolTransformArgs::LiquifyProperties::ROTATE: {
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
    case ToolTransformArgs::LiquifyProperties::OFFSET: {
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
    case ToolTransformArgs::LiquifyProperties::UNDO:
        break;
    }

    return outline;
}

KisSpacingInformation KisLiquifyPaintop::paintAt(const KisPaintInformation &pi)
{
    static const qreal sizeToSigmaCoeff = 1.0 / 3.0;
    const qreal size = sizeToSigmaCoeff *
        (m_d->props.sizeHasPressure() ?
         pi.pressure() * m_d->props.size():
         m_d->props.size());

    const qreal spacing = m_d->props.spacing() * size;

    const qreal reverseCoeff =
        m_d->props.currentMode() !=
        ToolTransformArgs::LiquifyProperties::UNDO &&
        m_d->props.reverseDirection() ? -1.0 : 1.0;
    const qreal amount = m_d->props.amountHasPressure() ?
        pi.pressure() * reverseCoeff * m_d->props.amount():
        reverseCoeff * m_d->props.amount();



    switch (m_d->props.currentMode()) {
    case ToolTransformArgs::LiquifyProperties::MOVE: {
        const qreal offsetLength = size * amount;
        m_d->worker->translatePoints(pi.pos(),
                                     pi.drawingDirectionVector() * offsetLength,
                                     size);

        break;
    }
    case ToolTransformArgs::LiquifyProperties::SCALE:
        m_d->worker->scalePoints(pi.pos(),
                                 amount,
                                 size);
        break;
    case ToolTransformArgs::LiquifyProperties::ROTATE:
        m_d->worker->rotatePoints(pi.pos(),
                                  2.0 * M_PI * amount,
                                  size);
        break;
    case ToolTransformArgs::LiquifyProperties::OFFSET: {
        const qreal offsetLength = size * amount;
        m_d->worker->translatePoints(pi.pos(),
                                     KisAlgebra2D::rightUnitNormal(pi.drawingDirectionVector()) * offsetLength,
                                     size);
        break;
    }
    case ToolTransformArgs::LiquifyProperties::UNDO:
        m_d->worker->undoPoints(pi.pos(),
                                amount,
                                size);

        break;
    }

    return KisSpacingInformation(spacing);
}
