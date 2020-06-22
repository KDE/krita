/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISMASKEDPAINTINGSTROKEDATA_H
#define KISMASKEDPAINTINGSTROKEDATA_H

#include "kritaui_export.h"

#include <QVector>
#include <QSharedPointer>

class KisFreehandStrokeInfo;
class KisPaintInformation;
class KisDistanceInformation;
class QPointF;
class QRectF;
class QRect;
class QPainterPath;
class QPen;
class KoColor;
class KisRunnableStrokeJobData;

class KisPaintOpPreset;
typedef QSharedPointer<KisPaintOpPreset> KisPaintOpPresetSP;


class KRITAUI_EXPORT KisMaskedFreehandStrokePainter
{
public:
    KisMaskedFreehandStrokePainter(KisFreehandStrokeInfo *strokeData, KisFreehandStrokeInfo *maskData);

    // painter overrides

    KisPaintOpPresetSP preset() const;

    void paintAt(const KisPaintInformation& pi);

    void paintLine(const KisPaintInformation &pi1,
                   const KisPaintInformation &pi2);

    void paintBezierCurve(const KisPaintInformation &pi1,
                          const QPointF &control1,
                          const QPointF &control2,
                          const KisPaintInformation &pi2);

    void paintPolyline(const QVector<QPointF> &points,
                       int index = 0, int numPoints = -1);

    void paintPolygon(const QVector<QPointF> &points);
    void paintRect(const QRectF &rect);
    void paintEllipse(const QRectF &rect);
    void paintPainterPath(const QPainterPath& path);

    void drawPainterPath(const QPainterPath& path, const QPen& pen);
    void drawAndFillPainterPath(const QPainterPath& path, const QPen& pen, const KoColor &customColor);

    // paintop overrides

    std::pair<int, bool> doAsyncronousUpdate(QVector<KisRunnableStrokeJobData*> &jobs);
    bool hasDirtyRegion() const;
    QVector<QRect> takeDirtyRegion();

    bool hasMasking() const;

private:
    template <class Func>
    inline void applyToAllPainters(Func func);

private:
    KisFreehandStrokeInfo *m_stroke = 0;
    KisFreehandStrokeInfo *m_mask = 0;
};

#endif // KISMASKEDPAINTINGSTROKEDATA_H
