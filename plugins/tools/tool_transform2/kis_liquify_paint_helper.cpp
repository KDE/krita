/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_liquify_paint_helper.h"

#include <QElapsedTimer>
#include <QPainterPath>

#include "kis_algebra_2d.h"
#include "KoPointerEvent.h"
#include <brushengine/kis_paint_information.h>
#include "kis_painting_information_builder.h"
#include "kis_liquify_transform_worker.h"
#include <brushengine/kis_paintop_utils.h>
#include "kis_coordinates_converter.h"
#include "kis_liquify_paintop.h"
#include "kis_liquify_properties.h"

struct KisLiquifyPaintHelper::Private
{
    Private(const KisCoordinatesConverter *_converter)
        : converter(_converter),
          infoBuilder(new KisConverterPaintingInformationBuilder(converter)),
          hasPaintedAtLeastOnce(false)
    {
    }

    KisPaintInformation previousPaintInfo;

    QScopedPointer<KisLiquifyPaintop> paintOp;
    KisDistanceInformation currentDistance;
    const KisCoordinatesConverter *converter;
    QScopedPointer<KisPaintingInformationBuilder> infoBuilder;

    QElapsedTimer strokeTime;

    bool hasPaintedAtLeastOnce;

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
    QPointF prevPos = lastOutlinePos.pushThroughHistory(info.pos(), converter->effectiveZoom());
    qreal angle = KisAlgebra2D::directionBetweenPoints(prevPos, info.pos(), 0);

    previousDistanceInfo =
        KisDistanceInformation(prevPos, angle);

    previousPaintInfo = info;
}

QPainterPath KisLiquifyPaintHelper::brushOutline(const KisLiquifyProperties &props)
{
    KisPaintInformation::DistanceInformationRegistrar registrar =
        m_d->previousPaintInfo.registerDistanceInformation(&m_d->previousDistanceInfo);

    return KisLiquifyPaintop::brushOutline(props, m_d->previousPaintInfo);
}

void KisLiquifyPaintHelper::configurePaintOp(const KisLiquifyProperties &props,
                                             KisLiquifyTransformWorker *worker)
{
    m_d->paintOp.reset(new KisLiquifyPaintop(props, worker));
}

void KisLiquifyPaintHelper::startPaint(KoPointerEvent *event, const KoCanvasResourceProvider *manager)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->paintOp);

    m_d->strokeTime.start();
    KisPaintInformation pi =
        m_d->infoBuilder->startStroke(event, m_d->strokeTime.elapsed(), manager);

    m_d->updatePreviousPaintInfo(pi);
    m_d->hasPaintedAtLeastOnce = false;
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
    m_d->hasPaintedAtLeastOnce = true;
}

bool KisLiquifyPaintHelper::endPaint(KoPointerEvent *event)
{
    KIS_ASSERT_RECOVER(m_d->paintOp) { return false; }

    if (!m_d->hasPaintedAtLeastOnce) {
        KisPaintInformation pi =
            m_d->infoBuilder->continueStroke(event, m_d->strokeTime.elapsed());

        pi.paintAt(*m_d->paintOp.data(), &m_d->previousDistanceInfo);
    }

    m_d->paintOp.reset();

    return !m_d->hasPaintedAtLeastOnce;
}

void KisLiquifyPaintHelper::hoverPaint(KoPointerEvent *event)
{
    QPointF imagePoint = m_d->converter->documentToImage(event->pos());
    KisPaintInformation pi = m_d->infoBuilder->hover(imagePoint, event, m_d->paintOp);

    m_d->updatePreviousPaintInfo(pi);
}
