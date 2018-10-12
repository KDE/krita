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

#ifndef KISDABRENDERINGEXECUTOR_H
#define KISDABRENDERINGEXECUTOR_H

#include "kritadefaultpaintops_export.h"

#include <QScopedPointer>

#include <QList>
struct KisRenderedDab;

#include "KisDabCacheUtils.h"

class KisPressureMirrorOption;
class KisPrecisionOption;
class KisRunnableStrokeJobsInterface;


class KRITADEFAULTPAINTOPS_EXPORT KisDabRenderingExecutor
{
public:
    KisDabRenderingExecutor(const KoColorSpace *cs,
                            KisDabCacheUtils::ResourcesFactory resourcesFactory,
                            KisRunnableStrokeJobsInterface *runnableJobsInterface,
                            KisPressureMirrorOption *mirrorOption = 0,
                            KisPrecisionOption *precisionOption = 0);
    ~KisDabRenderingExecutor();

    void addDab(const KisDabCacheUtils::DabRequestInfo &request,
                qreal opacity, qreal flow);

    QList<KisRenderedDab> takeReadyDabs(bool returnMutableDabs = false, int oneTimeLimit = -1, bool *someDabsLeft = 0);

    bool hasPreparedDabs() const;

    qreal averageDabRenderingTime() const; // msecs
    int averageDabSize() const;

private:
    KisDabRenderingExecutor(const KisDabRenderingExecutor &rhs) = delete;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISDABRENDERINGEXECUTOR_H
