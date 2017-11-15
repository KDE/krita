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
    KisWatershedWorker(KisPaintDeviceSP src,
                       KisPaintDeviceSP dst,
                       const QRect &boundingRect);
    ~KisWatershedWorker();

    void addKeyStroke(KisPaintDeviceSP dev, const KoColor &color);
    void run(bool doCleanUp = false);

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
