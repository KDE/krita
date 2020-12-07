/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CAGE_TRANSFORM_WORKER_H
#define __KIS_CAGE_TRANSFORM_WORKER_H

#include <QScopedPointer>
#include <kritaimage_export.h>
#include <kis_types.h>

class QImage;

class KRITAIMAGE_EXPORT KisCageTransformWorker
{
public:
    KisCageTransformWorker(KisPaintDeviceSP dev,
                           const QVector<QPointF> &origCage,
                           KoUpdater *progress,
                           int pixelPrecision = 8);

    KisCageTransformWorker(const QImage &srcImage,
                           const QPointF &srcImageOffset,
                           const QVector<QPointF> &origCage,
                           KoUpdater *progress,
                           int pixelPrecision = 8);

    ~KisCageTransformWorker();

    void prepareTransform();
    void setTransformedCage(const QVector<QPointF> &transformedCage);
    void run();

    QRect approxChangeRect(const QRect &rc);
    QRect approxNeedRect(const QRect &rc, const QRect &fullBounds);

    QImage runOnQImage(QPointF *newOffset);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_CAGE_TRANSFORM_WORKER_H */
