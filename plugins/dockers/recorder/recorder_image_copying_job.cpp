/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "recorder_image_copying_job.h"

RecorderImageCopyingJob::RecorderImageCopyingJob(KisPaintDeviceSP device,
                                                 const QRect &rect,
                                                 int writerId,
                                                 int partIndex,
                                                 QVector<QRect> regions)
    : m_device(device)
    , m_writerId(writerId)
    , m_partIndex(partIndex)
    , m_area(rect)
    , m_regions(regions)
{
}

RecorderImageCopyingJob::~RecorderImageCopyingJob()
{
}

void RecorderImageCopyingJob::run()
{
    m_outputDevice = new KisPaintDevice(m_device->colorSpace());
    m_outputDevice->makeCloneFromRough(m_device, m_area);

    Q_EMIT sigImageCopyingDoneWithData(m_writerId, m_partIndex, m_regions, m_outputDevice);
}

bool RecorderImageCopyingJob::overrides(const KisSpontaneousJob *_otherJob)
{
    Q_UNUSED(_otherJob);
    return false;
}

QString RecorderImageCopyingJob::debugName() const
{
    return "KisImageCopyingJobBase";
}
