/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_colorize_job.h"

#include <QBitArray>


#include "kis_paint_device.h"
#include "kis_lazy_fill_tools.h"
#include "kis_gaussian_kernel.h"
#include "kis_painter.h"

using namespace KisLazyFillTools;

struct KisColorizeJob::Private
{
    KisPaintDeviceSP src;
    KisPaintDeviceSP dst;
    KisPaintDeviceSP filteredSource;
    QRect boundingRect;

    QVector<KeyStroke> keyStrokes;
};

KisColorizeJob::KisColorizeJob(KisPaintDeviceSP src,
                               KisPaintDeviceSP dst,
                               KisPaintDeviceSP filteredSource,
                               const QRect &boundingRect)
    : m_d(new Private)
{
    m_d->src = src;
    m_d->dst = dst;
    m_d->filteredSource = filteredSource;
    m_d->boundingRect = boundingRect;
}

KisColorizeJob::~KisColorizeJob()
{
}

void KisColorizeJob::addKeyStroke(KisPaintDeviceSP dev, const KoColor &color)
{
    KoColor convertedColor(color);
    convertedColor.convertTo(m_d->dst->colorSpace());

    m_d->keyStrokes << KeyStroke(dev, convertedColor);
}

void KisColorizeJob::run()
{
    const QRect &rect = m_d->boundingRect;

    // for b/w + noalpha layer
    //KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(m_d->src);

    // for alpha-based layers
    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsAlpha(m_d->src);

    // optional filtering
    // KisGaussianKernel::applyLoG(filteredMainDev,
    //                             m_d->boundingRect,
    //                             1.0,
    //                             QBitArray(), 0);

    normalizeAndInvertAlpha8Device(filteredMainDev, m_d->boundingRect);

    KisMultiwayCut cut(filteredMainDev, m_d->dst, m_d->boundingRect);

    Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
        cut.addKeyStroke(new KisPaintDevice(*stroke.dev), stroke.color);
    }

    cut.run();

    m_d->filteredSource->makeCloneFrom(filteredMainDev, m_d->boundingRect);

    emit sigFinished();
}

bool KisColorizeJob::overrides(const KisSpontaneousJob *_otherJob)
{
    const KisColorizeJob *otherJob =
        dynamic_cast<const KisColorizeJob*>(_otherJob);

    return otherJob->m_d->src == m_d->src &&
        otherJob->m_d->dst == m_d->dst;
}

int KisColorizeJob::levelOfDetail() const
{
    return 0;
}
