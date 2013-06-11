/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_filter_stroke_strategy.h"

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_transaction.h>


struct KisFilterStrokeStrategy::Private {
    KisFilterSP filter;
    KisSafeFilterConfigurationSP filterConfig;
    KisNodeSP node;
    KisUpdatesFacade *updatesFacade;

    bool cancelSilently;
    KisPaintDeviceSP filterDevice;
    KisTransaction *secondaryTransaction;
};


KisFilterStrokeStrategy::KisFilterStrokeStrategy(KisFilterSP filter,
                                                 KisSafeFilterConfigurationSP filterConfig,
                                                 KisResourcesSnapshotSP resources)
    : KisPainterBasedStrokeStrategy("FILTER_STROKE", filter->name(), resources,
                                    QVector<PainterInfo*>()),
      m_d(new Private())
{
    m_d->filter = filter;
    m_d->filterConfig = filterConfig;
    m_d->node = resources->currentNode();
    m_d->updatesFacade = resources->image().data();
    m_d->cancelSilently = false;

    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

KisFilterStrokeStrategy::~KisFilterStrokeStrategy()
{
    delete m_d;
}

void KisFilterStrokeStrategy::initStrokeCallback()
{
    KisPainterBasedStrokeStrategy::initStrokeCallback();

    KisPaintDeviceSP dev = targetDevice();

    if (activeSelection() ||
        (dev->colorSpace() != dev->compositionSourceColorSpace() &&
         !(dev->colorSpace() == dev->compositionSourceColorSpace()))) {

        m_d->filterDevice = dev->createCompositionSourceDevice(dev);
        m_d->secondaryTransaction = new KisTransaction("", m_d->filterDevice);
    } else {
        m_d->filterDevice = dev;
    }
}

void KisFilterStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);
    CancelSilentlyMarker *cancelJob =
        dynamic_cast<CancelSilentlyMarker*>(data);

    if (d) {
        QRect rc = d->processRect;

        KisProcessingVisitor::ProgressHelper helper(m_d->node);

        m_d->filter->processImpl(m_d->filterDevice, rc,
                                 m_d->filterConfig.data(), helper.updater());

        if (m_d->secondaryTransaction) {
            KisPainter p(targetDevice());
            p.setCompositeOp(COMPOSITE_COPY);
            p.setSelection(activeSelection());
            p.bitBlt(rc.topLeft(), m_d->filterDevice, rc);

            // Free memory
            m_d->filterDevice->clear(rc);
        }

        m_d->node->setDirty(rc);
    } else if (cancelJob) {
        m_d->cancelSilently = true;
    } else {
        qFatal("KisFilterStrokeStrategy: job type is not known");
    }
}

void KisFilterStrokeStrategy::cancelStrokeCallback()
{
    delete m_d->secondaryTransaction;
    m_d->filterDevice = 0;

    if (m_d->cancelSilently) {
        m_d->updatesFacade->disableDirtyRequests();
    }

    KisPainterBasedStrokeStrategy::cancelStrokeCallback();

    if (m_d->cancelSilently) {
        m_d->updatesFacade->enableDirtyRequests();
    }
}

void KisFilterStrokeStrategy::finishStrokeCallback()
{
    delete m_d->secondaryTransaction;
    m_d->filterDevice = 0;

    KisPainterBasedStrokeStrategy::finishStrokeCallback();
}
