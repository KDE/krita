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

#include "kis_liquify_paint_helper.h"

#include "KoPointerEvent.h"
#include "kis_paint_information.h"
#include "kis_painting_information_builder.h"
#include "kis_liquify_transform_worker.h"
#include "kis_paintop_utils.h"
#include "kis_coordinates_converter.h"
#include "kis_liquify_paintop.h"
#include "kis_paintop_utils.h"


struct KisLiquifyPaintHelper::Private
{
    Private(const KisCoordinatesConverter *_converter)
        : converter(_converter),
          infoBuilder(new KisConverterPaintingInformationBuilder(converter))
    {
    }

    KisPaintInformation previousPaintInfo;

    QScopedPointer<KisLiquifyPaintop> paintOp;
    KisDistanceInformation currentDistance;
    const KisCoordinatesConverter *converter;
    QScopedPointer<KisPaintingInformationBuilder> infoBuilder;

    QTime strokeTime;

    KisDistanceInformation previousDistanceInfo;
    KisPaintOpUtils::PositionHistory lastOutlinePos;
    void updatePreviousPaintInfo(const KisPaintInformation &info);
};


KisLiquifyPaintHelper::KisLiquifyPaintHelper(const KisCoordinatesConverter *converter)
    : m_d(new Private(converter))
{
}

KisLiquifyPaintHelper::~KisLiquifyPaintHelper()
{
}

void KisLiquifyPaintHelper::Private::updatePreviousPaintInfo(const KisPaintInformation &info)
{
    previousDistanceInfo =
        KisDistanceInformation(
            lastOutlinePos.pushThroughHistory(info.pos()), 0);

    previousPaintInfo = info;
}

QPainterPath KisLiquifyPaintHelper::brushOutline(const ToolTransformArgs::LiquifyProperties &props) const
{
    KisPaintInformation::DistanceInformationRegistrar registrar =
        m_d->previousPaintInfo.registerDistanceInformation(&m_d->previousDistanceInfo);

    return KisLiquifyPaintop::brushOutline(props, m_d->previousPaintInfo);
}

void KisLiquifyPaintHelper::configurePaintOp(const ToolTransformArgs::LiquifyProperties &props,
                                             KisLiquifyTransformWorker *worker)
{
    m_d->paintOp.reset(new KisLiquifyPaintop(props, worker));
}

void KisLiquifyPaintHelper::startPaint(KoPointerEvent *event)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->paintOp);

    m_d->strokeTime.start();
    KisPaintInformation pi =
        m_d->infoBuilder->startStroke(event, m_d->strokeTime.elapsed());

    m_d->updatePreviousPaintInfo(pi);
}

void KisLiquifyPaintHelper::continuePaint(KoPointerEvent *event)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->paintOp);

    KisPaintInformation pi =
        m_d->infoBuilder->continueStroke(event, m_d->strokeTime.elapsed());

    KisPaintOpUtils::paintLine(*m_d->paintOp.data(),
                               m_d->previousPaintInfo,
                               pi,
                               &m_d->currentDistance,
                               false, false);

    m_d->updatePreviousPaintInfo(pi);
}

void KisLiquifyPaintHelper::endPaint(KoPointerEvent *event)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->paintOp);

    continuePaint(event);
    m_d->paintOp.reset();
}

void KisLiquifyPaintHelper::hoverPaint(KoPointerEvent *event)
{
    QPointF imagePoint = m_d->converter->documentToImage(event->pos());
    KisPaintInformation pi = m_d->infoBuilder->hover(imagePoint, event);

    m_d->updatePreviousPaintInfo(pi);
}
