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

#ifndef KISDABRENDERINGJOB_H
#define KISDABRENDERINGJOB_H

#include <QRunnable>
#include <KisDabCacheUtils.h>
#include <kis_fixed_paint_device.h>
#include <kis_types.h>
#include "kritadefaultpaintops_export.h"

class KisDabRenderingQueue;
class KisRunnableStrokeJobsInterface;

class KRITADEFAULTPAINTOPS_EXPORT KisDabRenderingJob
{
public:
    enum JobType {
        Dab,
        Postprocess,
        Copy
    };

    enum Status {
        New,
        Running,
        Completed
    };

public:
    KisDabRenderingJob();
    KisDabRenderingJob(int _seqNo,
                       KisDabCacheUtils::DabGenerationInfo _generationInfo,
                       JobType _type);
    KisDabRenderingJob(const KisDabRenderingJob &rhs);
    KisDabRenderingJob& operator=(const KisDabRenderingJob &rhs);

    QPoint dstDabOffset() const;

    int seqNo = -1;
    KisDabCacheUtils::DabGenerationInfo generationInfo;
    JobType type = Dab;
    KisFixedPaintDeviceSP originalDevice;
    KisFixedPaintDeviceSP postprocessedDevice;

    // high-level members, not directly related to job execution itself
    Status status = New;

    qreal opacity = OPACITY_OPAQUE_F;
    qreal flow = OPACITY_OPAQUE_F;
};

#include <QSharedPointer>
typedef QSharedPointer<KisDabRenderingJob> KisDabRenderingJobSP;

class KRITADEFAULTPAINTOPS_EXPORT KisDabRenderingJobRunner : public QRunnable
{
public:
    KisDabRenderingJobRunner(KisDabRenderingJobSP job,
                             KisDabRenderingQueue *parentQueue,
                             KisRunnableStrokeJobsInterface *runnableJobsInterface);
    ~KisDabRenderingJobRunner();

    void run() override;

    static int executeOneJob(KisDabRenderingJob *job, KisDabCacheUtils::DabRenderingResources *resources, KisDabRenderingQueue *parentQueue);

private:
    KisDabRenderingJobSP m_job;
    KisDabRenderingQueue *m_parentQueue = 0;
    KisRunnableStrokeJobsInterface *m_runnableJobsInterface = 0;
};


#endif // KISDABRENDERINGJOB_H
