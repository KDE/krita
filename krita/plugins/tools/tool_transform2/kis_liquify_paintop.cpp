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

KisSpacingInformation KisLiquifyPaintop::paintAt(const KisPaintInformation &pi)
{
    const qreal size = m_d->props.sizeHasPressure() ?
        pi.pressure() * m_d->props.size():
        m_d->props.size();

    const qreal spacing = m_d->props.spacing() * size;

    const qreal reverseCoeff = m_d->props.reverseDirection() ? -1.0 : 1.0;
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
