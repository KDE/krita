/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_STROKES_QUEUE_H
#define __KIS_STROKES_QUEUE_H

#include "krita_export.h"
#include "kis_types.h"
#include "kis_stroke_job_strategy.h"
#include "kis_stroke_strategy.h"

class KisUpdaterContext;
class KisStroke;

class KRITAIMAGE_EXPORT KisStrokesQueue
{
public:
    KisStrokesQueue();
    ~KisStrokesQueue();

    KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy);
    void addJob(KisStrokeId id, KisStrokeJobData *data);

    void endStroke(KisStrokeId id);
    bool cancelStroke(KisStrokeId id);

    void processQueue(KisUpdaterContext &updaterContext, bool externalJobsPending);
    bool needsExclusiveAccess() const;
    bool isEmpty() const;

    qint32 sizeMetric() const;
    QString currentStrokeName() const;

private:
    bool processOneJob(KisUpdaterContext &updaterContext, bool externalJobsPending);
    bool checkStrokeState(bool hasStrokeJobsRunning);
    bool checkExclusiveProperty(qint32 numMergeJobs, qint32 numStrokeJobs);
    bool checkSequentialProperty(qint32 numMergeJobs, qint32 numStrokeJobs);
    bool checkBarrierProperty(qint32 numMergeJobs, qint32 numStrokeJobs,
                              bool externalJobsPending);
private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_STROKES_QUEUE_H */
