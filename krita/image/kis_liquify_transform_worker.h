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

#ifndef __KIS_LIQUIFY_TRANSFORM_WORKER_H
#define __KIS_LIQUIFY_TRANSFORM_WORKER_H

#include <QScopedPointer>
#include <krita_export.h>
#include <kis_types.h>

class QImage;

class KRITAIMAGE_EXPORT KisLiquifyTransformWorker
{
public:
    KisLiquifyTransformWorker(const QRect &srcBounds,
                              KoUpdater *progress,
                              int pixelPrecision = 8);

    KisLiquifyTransformWorker(const KisLiquifyTransformWorker &rhs);

    ~KisLiquifyTransformWorker();

    int pointToIndex(const QPoint &cellPt);
    QSize gridSize() const;

    void translatePoints(const QPointF &base,
                         const QPointF &offset,
                         qreal sigma);

    void scalePoints(const QPointF &base,
                     qreal scale,
                     qreal sigma);

    void rotatePoints(const QPointF &base,
                      qreal angle,
                      qreal sigma);

    void undoPoints(const QPointF &base,
                    qreal amount,
                    qreal sigma);

    const QVector<QPointF>& originalPoints() const;
    QVector<QPointF>& transformedPoints();

    void run(KisPaintDeviceSP device);
    QImage runOnQImage(const QImage &srcImage,
                       const QPointF &srcImageOffset,
                       const QTransform &imageToThumbTransform,
                       QPointF *newOffset);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LIQUIFY_TRANSFORM_WORKER_H */
