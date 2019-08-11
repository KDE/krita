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

#ifndef KISWATERSHEDWORKER_H
#define KISWATERSHEDWORKER_H

#include <QScopedPointer>

#include "kis_types.h"
#include "kritaimage_export.h"

class KoColor;

class KRITAIMAGE_EXPORT KisWatershedWorker
{
public:
    /**
     * Creates an empty watershed worker without any strokes attached. The strokes
     * should be attached manually with addKeyStroke() call.
     *
     * @param heightMap prefiltered height map in alpha8 colorspace, with "0" meaning
     *                  background color and "255" meaning line art. Heightmap is *never*
     *                  modified by the worker!
     * @param dst destination device where the result will be written
     * @param boundingRect the worker refuses to fill outside the bounding rect, considering
     *                     that outer area as having +inf height
     * @param progress  the progress value
     */
    KisWatershedWorker(KisPaintDeviceSP heightMap,
                       KisPaintDeviceSP dst,
                       const QRect &boundingRect,
                       KoUpdater *progress = 0);
    ~KisWatershedWorker();

    /**
     * @brief Adds a key stroke to the worker.
     *
     * The key strokes may intersect, in which case the lastly added stroke will have
     * a priority over all the previous ones.
     *
     * @param dev alpha8 paint device of the key stroke, may contain disjoint areas
     * @param color the color of the stroke
     */
    void addKeyStroke(KisPaintDeviceSP dev, const KoColor &color);

    /**
     * @brief run the filling process using the passes height map, strokes, and write
     *        the result coloring into the destination device
     * @param cleanUpAmount shows how aggressively we should try to clean up the final
     *                      coloring. Should be in range [0.0...1.0]
     */

    void run(qreal cleanUpAmount = 0.0);

    int testingGroupPositiveEdge(qint32 group, quint8 level);
    int testingGroupNegativeEdge(qint32 group, quint8 level);
    int testingGroupForeignEdge(qint32 group, quint8 level);
    int testingGroupAllyEdge(qint32 group, quint8 level);
    int testingGroupConflicts(qint32 group, quint8 level, qint32 withGroup);

    void testingTryRemoveGroup(qint32 group, quint8 level);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISWATERSHEDWORKER_H
