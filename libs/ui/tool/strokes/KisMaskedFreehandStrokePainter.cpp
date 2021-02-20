/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMaskedFreehandStrokePainter.h"

#include "kis_assert.h"
#include "kis_painter.h"
#include "KisFreehandStrokeInfo.h"
#include "kis_paintop.h"
#include "kis_paintop_preset.h"


KisMaskedFreehandStrokePainter::KisMaskedFreehandStrokePainter(KisFreehandStrokeInfo *strokeData, KisFreehandStrokeInfo *maskData)
    : m_stroke(strokeData),
      m_mask(maskData)
{
}

KisPaintOpPresetSP KisMaskedFreehandStrokePainter::preset() const
{
    return m_stroke->painter->preset();
}

template <class Func>
void KisMaskedFreehandStrokePainter::applyToAllPainters(Func func)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_stroke);
    func(m_stroke);

    if (m_mask) {
        func(m_mask);
    }
}

void KisMaskedFreehandStrokePainter::paintAt(const KisPaintInformation &pi)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintAt(pi, data->dragDistance);
    });
}

void KisMaskedFreehandStrokePainter::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintLine(pi1, pi2, data->dragDistance);
    });
}

void KisMaskedFreehandStrokePainter::paintBezierCurve(const KisPaintInformation &pi1, const QPointF &control1, const QPointF &control2, const KisPaintInformation &pi2)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintBezierCurve(pi1, control1, control2, pi2, data->dragDistance);
    });
}

void KisMaskedFreehandStrokePainter::paintPolyline(const QVector<QPointF> &points, int index, int numPoints)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintPolyline(points, index, numPoints);
    });
}

void KisMaskedFreehandStrokePainter::paintPolygon(const QVector<QPointF> &points)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintPolygon(points);
    });
}

void KisMaskedFreehandStrokePainter::paintRect(const QRectF &rect)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintRect(rect);
    });
}

void KisMaskedFreehandStrokePainter::paintEllipse(const QRectF &rect)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintEllipse(rect);
    });
}

void KisMaskedFreehandStrokePainter::paintPainterPath(const QPainterPath &path)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->paintPainterPath(path);
    });
}

void KisMaskedFreehandStrokePainter::drawPainterPath(const QPainterPath &path, const QPen &pen)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->drawPainterPath(path, pen);
    });
}

void KisMaskedFreehandStrokePainter::drawAndFillPainterPath(const QPainterPath &path, const QPen &pen, const KoColor &customColor)
{
    applyToAllPainters([&] (KisFreehandStrokeInfo *data) {
        data->painter->setBackgroundColor(customColor);
        data->painter->fillPainterPath(path);
        data->painter->drawPainterPath(path, pen);
    });
}

std::pair<int, bool> KisMaskedFreehandStrokePainter::doAsyncronousUpdate(QVector<KisRunnableStrokeJobData *> &jobs)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_stroke);

    std::pair<int, bool> result =
        m_stroke->painter->paintOp()->doAsyncronousUpdate(jobs);

    if (m_mask) {
        QVector<KisRunnableStrokeJobData*> maskJobs;
        std::pair<int, bool> maskMetrics =
            m_mask->painter->paintOp()->doAsyncronousUpdate(maskJobs);

        result.first = std::max(result.first, maskMetrics.first);
        result.second = result.second | maskMetrics.second;

        jobs.append(maskJobs);
    }

    return result;
}

bool KisMaskedFreehandStrokePainter::hasDirtyRegion() const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_stroke);

    bool result = m_stroke->painter->hasDirtyRegion();

    if (m_mask) {
        result |= m_mask->painter->hasDirtyRegion();
    }

    return result;
}

QVector<QRect> KisMaskedFreehandStrokePainter::takeDirtyRegion()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_stroke);

    QVector<QRect> result = m_stroke->painter->takeDirtyRegion();

    if (m_mask) {
        result += m_mask->painter->takeDirtyRegion();
    }

    return result;
}

bool KisMaskedFreehandStrokePainter::hasMasking() const
{
    return m_mask;
}

