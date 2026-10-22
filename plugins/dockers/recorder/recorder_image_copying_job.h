/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RECORDER_IMAGE_COPYING_JOB_H 
#define RECORDER_IMAGE_COPYING_JOB_H 

#include "kis_paint_device.h"
#include "kis_spontaneous_job.h"
#include <qcontainerfwd.h>

class RecorderImageCopyingJob : public QObject, public KisSpontaneousJob
{
    Q_OBJECT

public:
    RecorderImageCopyingJob(KisPaintDeviceSP device, const QRect &rect, int index, QVector<QRect> regions);

    ~RecorderImageCopyingJob();
    void run() override;
    bool overrides(const KisSpontaneousJob *otherJob) override;

    QString debugName() const override;
    int levelOfDetail() const override
    {
        return 0;
    }

Q_SIGNALS:
    void sigImageCopyingDoneWithData(int index, QVector<QRect> regions, KisPaintDeviceSP device);

private:
    KisPaintDeviceSP m_device;
    KisPaintDeviceSP m_outputDevice;
    int m_partIndex;
    QRect m_area;
    QVector<QRect> m_regions;
};
#endif
