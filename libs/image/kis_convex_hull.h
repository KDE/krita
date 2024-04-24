#ifndef __KIS_CONVEX_HULL_H
#define __KIS_CONVEX_HULL_H

#include <QtGlobal>
#include <QPolygon>
#include "kis_types.h"

namespace KisConvexHull
{
    KRITAIMAGE_EXPORT QPolygon findConvexHull(const QVector<QPoint> &points);
    KRITAIMAGE_EXPORT QPolygon findConvexHull(KisPaintDeviceSP device);
}

#endif /* __KIS_CONVEX_HULL_H */