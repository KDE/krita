/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cage_transform_strategy.h"
#include "tool_transform_args.h"

#include <QPointF>
#include <QPainter>

#include "kis_painting_tweaks.h"
#include "kis_cursor.h"
#include <kis_cage_transform_worker.h>


struct KisCageTransformStrategy::Private
{
    Private(KisCageTransformStrategy *_q)
        : q(_q)
    {
    }

    KisCageTransformStrategy * const q;
};


KisCageTransformStrategy::KisCageTransformStrategy(const KisCoordinatesConverter *converter,
                                                   KoSnapGuide *snapGuide,
                                                   ToolTransformArgs &currentArgs,
                                                   TransformTransactionProperties &transaction)
    : KisWarpTransformStrategy(converter, snapGuide, currentArgs, transaction),
      m_d(new Private(this))
{
    overrideDrawingItems(true, false, true);
    setCloseOnStartPointClick(true);
    setClipOriginalPointsPosition(false);
    setTransformType(TransformType::CAGE_TRANSFORM);

}

KisCageTransformStrategy::~KisCageTransformStrategy()
{
}

void KisCageTransformStrategy::drawConnectionLines(QPainter &gc,
                                                   const QVector<QPointF> &origPoints,
                                                   const QVector<QPointF> &transfPoints,
                                                   bool isEditingPoints)
{
    const int numPoints = origPoints.size();
    if (numPoints <= 1) return;

    QPen antsPen;
    QPen outlinePen;

    KisPaintingTweaks::initAntsPen(&antsPen, &outlinePen);

    const int iterateLimit = isEditingPoints ? numPoints : numPoints + 1;

    for (int i = 1; i < iterateLimit; ++i) {
        int idx = i % numPoints;
        int prevIdx = (i - 1) % numPoints;

        gc.setPen(outlinePen);
        gc.drawLine(transfPoints[prevIdx], transfPoints[idx]);
        gc.setPen(antsPen);
        gc.drawLine(transfPoints[prevIdx], transfPoints[idx]);
    }
}

QImage KisCageTransformStrategy::calculateTransformedImage(ToolTransformArgs &currentArgs,
                                                           const QImage &srcImage,
                                                           const QVector<QPointF> &origPoints,
                                                           const QVector<QPointF> &transfPoints,
                                                           const QPointF &srcOffset,
                                                           QPointF *dstOffset)
{
    KisCageTransformWorker worker(srcImage,
                                  srcOffset,
                                  origPoints,
                                  0,
                                  currentArgs.previewPixelPrecision());
    worker.prepareTransform();
    worker.setTransformedCage(transfPoints);
    return worker.runOnQImage(dstOffset);
}
