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
#include <KoCompositeOpRegistry.h>


struct KisFilterStrokeStrategy::Private {
    Private()
        : updatesFacade(0),
          cancelSilently(false),
          secondaryTransaction(0),
          levelOfDetail(0)
    {
    }

    Private(const Private &rhs)
        : filter(rhs.filter),
          filterConfig(rhs.filterConfig),
          node(rhs.node),
          updatesFacade(rhs.updatesFacade),
          cancelSilently(rhs.cancelSilently),
          filterDevice(),
          filterDeviceBounds(),
          secondaryTransaction(0),
          progressHelper(),
          levelOfDetail(0)
    {
        KIS_ASSERT_RECOVER_RETURN(!rhs.filterDevice);
        KIS_ASSERT_RECOVER_RETURN(rhs.filterDeviceBounds.isEmpty());
        KIS_ASSERT_RECOVER_RETURN(!rhs.secondaryTransaction);
        KIS_ASSERT_RECOVER_RETURN(!rhs.progressHelper);
        KIS_ASSERT_RECOVER_RETURN(!rhs.levelOfDetail);
    }

    KisFilterSP filter;
    KisFilterConfigurationSP filterConfig;
    KisNodeSP node;
    KisUpdatesFacade *updatesFacade;

    bool cancelSilently;
    KisPaintDeviceSP filterDevice;
    QRect filterDeviceBounds;
    KisTransaction *secondaryTransaction;
    QScopedPointer<KisProcessingVisitor::ProgressHelper> progressHelper;

    int levelOfDetail;
};


KisFilterStrokeStrategy::KisFilterStrokeStrategy(KisFilterSP filter,
                                                 KisFilterConfigurationSP filterConfig,
                                                 KisResourcesSnapshotSP resources)
    : KisPainterBasedStrokeStrategy(QLatin1String("FILTER_STROKE"),
                                    kundo2_i18n("Filter \"%1\"", filter->name()),
                                    resources,
                                    QVector<KisFreehandStrokeInfo*>(),false),
      m_d(new Private())
{
    m_d->filter = filter;
    m_d->filterConfig = filterConfig;
    m_d->node = resources->currentNode();
    m_d->updatesFacade = resources->image().data();
    m_d->cancelSilently = false;
    m_d->secondaryTransaction = 0;
    m_d->levelOfDetail = 0;

    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

KisFilterStrokeStrategy::KisFilterStrokeStrategy(const KisFilterStrokeStrategy &rhs, int levelOfDetail)
    : KisPainterBasedStrokeStrategy(rhs, levelOfDetail),
      m_d(new Private(*rhs.m_d))
{
    // only non-started transaction are allowed
    KIS_ASSERT_RECOVER_NOOP(!m_d->secondaryTransaction);
    m_d->levelOfDetail = levelOfDetail;
}

KisFilterStrokeStrategy::~KisFilterStrokeStrategy()
{
    delete m_d;
}

void KisFilterStrokeStrategy::initStrokeCallback()
{
    KisPainterBasedStrokeStrategy::initStrokeCallback();

    KisPaintDeviceSP dev = targetDevice();
    m_d->filterDeviceBounds = dev->extent();


    if (activeSelection() ||
        (dev->colorSpace() != dev->compositionSourceColorSpace() &&
         *dev->colorSpace() != *dev->compositionSourceColorSpace())) {

        m_d->filterDevice = dev->createCompositionSourceDevice(dev);
        m_d->secondaryTransaction = new KisTransaction(m_d->filterDevice);

        if (activeSelection()) {
            m_d->filterDeviceBounds &= activeSelection()->selectedRect();
        }
    } else {
        m_d->filterDevice = dev;
    }

    m_d->progressHelper.reset(new KisProcessingVisitor::ProgressHelper(m_d->node));
}

void KisFilterStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);
    CancelSilentlyMarker *cancelJob =
        dynamic_cast<CancelSilentlyMarker*>(data);

    if (d) {
        const QRect rc = d->processRect;

        if (!m_d->filterDeviceBounds.intersects(
                m_d->filter->neededRect(rc, m_d->filterConfig.data(), m_d->levelOfDetail))) {

            return;
        }

        m_d->filter->processImpl(m_d->filterDevice, rc,
                                 m_d->filterConfig.data(),
                                 m_d->progressHelper->updater());

        if (m_d->secondaryTransaction) {
            KisPainter::copyAreaOptimized(rc.topLeft(), m_d->filterDevice, targetDevice(), rc, activeSelection());

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

    KisProjectionUpdatesFilterSP prevUpdatesFilter;

    if (m_d->cancelSilently) {
        /**
         * TODO: Projection updates filter is not recursive, please
         *       redesign this part
         */
        prevUpdatesFilter = m_d->updatesFacade->projectionUpdatesFilter();
        if (prevUpdatesFilter) {
            m_d->updatesFacade->setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP());
        }
        m_d->updatesFacade->disableDirtyRequests();
    }

    KisPainterBasedStrokeStrategy::cancelStrokeCallback();

    if (m_d->cancelSilently) {
        m_d->updatesFacade->enableDirtyRequests();
        if (prevUpdatesFilter) {
            m_d->updatesFacade->setProjectionUpdatesFilter(prevUpdatesFilter);
            prevUpdatesFilter.clear();
        }
    }
}

void KisFilterStrokeStrategy::finishStrokeCallback()
{
    delete m_d->secondaryTransaction;
    m_d->filterDevice = 0;

    KisPainterBasedStrokeStrategy::finishStrokeCallback();
}

KisStrokeStrategy* KisFilterStrokeStrategy::createLodClone(int levelOfDetail)
{
    if (!m_d->filter->supportsLevelOfDetail(m_d->filterConfig.data(), levelOfDetail)) return 0;

    KisFilterStrokeStrategy *clone = new KisFilterStrokeStrategy(*this, levelOfDetail);
    return clone;
}
