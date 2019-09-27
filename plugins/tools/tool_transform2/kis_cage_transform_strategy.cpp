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
                                                   ToolTransformArgs &currentArgs,
                                                   TransformTransactionProperties &transaction)
    : KisWarpTransformStrategy(converter, currentArgs, transaction),
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
