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

#include <KisSharedRunnable.h>
#include <KisDabCacheUtils.h>
#include <kis_fixed_paint_device.h>
#include <kis_types.h>

class KisDabRenderingQueue;
class QThreadPool;
class KisSharedThreadPoolAdapter;

class KisDabRenderingJob : public KisSharedRunnable
{
public:
    enum JobType {
        Dab,
        Postprocess,
        Copy
    };

public:
    KisDabRenderingJob();
    KisDabRenderingJob(int _seqNo,
                       KisDabCacheUtils::DabGenerationInfo _generationInfo,
                       KisDabCacheUtils::DabRenderingResources *_resources,
                       JobType _type);
    KisDabRenderingJob(const KisDabRenderingJob &rhs);
    KisDabRenderingJob& operator=(const KisDabRenderingJob &rhs);


    void runShared() override;

    QPoint dstDabOffset() const;

    int seqNo = -1;
    KisDabCacheUtils::DabGenerationInfo generationInfo;
    KisDabCacheUtils::DabRenderingResources *resources;
    JobType type = Dab;
    KisFixedPaintDeviceSP originalDevice;
    KisFixedPaintDeviceSP postprocessedDevice;
    KisDabRenderingQueue *parentQueue = 0;
    KisSharedThreadPoolAdapter *sharedThreadPool = 0;

private:
    static int executeOneJob(KisDabRenderingJob *job);
};

#endif // KISDABRENDERINGJOB_H
