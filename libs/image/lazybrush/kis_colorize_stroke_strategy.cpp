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

#include "kis_colorize_stroke_strategy.h"

#include <QBitArray>

#include "krita_utils.h"
#include "kis_paint_device.h"
#include "kis_lazy_fill_tools.h"
#include "kis_gaussian_kernel.h"
#include "kis_painter.h"
#include "kis_default_bounds_base.h"
#include "kis_lod_transform.h"
#include "kis_node.h"
#include "kis_image_config.h"
#include "KisWatershedWorker.h"
#include "kis_processing_visitor.h"

#include "kis_transaction.h"

#include <KisRunnableStrokeJobData.h>
#include <KisRunnableStrokeJobUtils.h>
#include <KisRunnableStrokeJobsInterface.h>

using namespace KisLazyFillTools;

struct KisColorizeStrokeStrategy::Private
{
    Private() : filteredSourceValid(false) {}
    Private(const Private &rhs, int _levelOfDetail)
        : progressNode(rhs.progressNode)
        , src(rhs.src)
        , dst(rhs.dst)
        , filteredSource(rhs.filteredSource)
        , internalFilteredSource(rhs.internalFilteredSource)
        , filteredSourceValid(rhs.filteredSourceValid)
        , boundingRect(rhs.boundingRect)
        , prefilterOnly(rhs.prefilterOnly)
        , levelOfDetail(_levelOfDetail)
        , keyStrokes(rhs.keyStrokes)
        , filteringOptions(rhs.filteringOptions)
    {}

    KisNodeSP progressNode;
    KisPaintDeviceSP src;
    KisPaintDeviceSP dst;
    KisPaintDeviceSP filteredSource;
    KisPaintDeviceSP heightMap;
    KisPaintDeviceSP internalFilteredSource;
    bool filteredSourceValid;
    QRect boundingRect;

    bool prefilterOnly = false;
    int levelOfDetail = 0;

    QVector<KeyStroke> keyStrokes;

    // default values: disabled
    FilteringOptions filteringOptions;
};

KisColorizeStrokeStrategy::KisColorizeStrokeStrategy(KisPaintDeviceSP src,
                                                     KisPaintDeviceSP dst,
                                                     KisPaintDeviceSP filteredSource,
                                                     bool filteredSourceValid,
                                                     const QRect &boundingRect,
                                                     KisNodeSP progressNode,
                                                     bool prefilterOnly)
    : KisRunnableBasedStrokeStrategy("colorize-stroke", prefilterOnly ? kundo2_i18n("Prefilter Colorize Mask") : kundo2_i18n("Colorize")),
      m_d(new Private)
{
    m_d->progressNode = progressNode;
    m_d->src = src;
    m_d->dst = dst;
    m_d->filteredSource = filteredSource;
    m_d->boundingRect = boundingRect;
    m_d->filteredSourceValid = filteredSourceValid;
    m_d->prefilterOnly = prefilterOnly;

    enableJob(JOB_INIT, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    enableJob(JOB_DOSTROKE, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    enableJob(JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    setNeedsExplicitCancel(true);
}

KisColorizeStrokeStrategy::KisColorizeStrokeStrategy(const KisColorizeStrokeStrategy &rhs, int levelOfDetail)
    : KisRunnableBasedStrokeStrategy(rhs),
      m_d(new Private(*rhs.m_d, levelOfDetail))
{
    KisLodTransform t(levelOfDetail);
    m_d->boundingRect = t.map(rhs.m_d->boundingRect);
}

KisColorizeStrokeStrategy::~KisColorizeStrokeStrategy()
{
}

void KisColorizeStrokeStrategy::setFilteringOptions(const FilteringOptions &value)
{
    m_d->filteringOptions = value;
}

FilteringOptions KisColorizeStrokeStrategy::filteringOptions() const
{
    return m_d->filteringOptions;
}

void KisColorizeStrokeStrategy::addKeyStroke(KisPaintDeviceSP dev, const KoColor &color)
{
    KoColor convertedColor(color);
    convertedColor.convertTo(m_d->dst->colorSpace());

    m_d->keyStrokes << KeyStroke(dev, convertedColor);
}

void KisColorizeStrokeStrategy::initStrokeCallback()
{
    using namespace KritaUtils;

    QVector<KisRunnableStrokeJobData*> jobs;

    const QVector<QRect> patchRects =
        splitRectIntoPatches(m_d->boundingRect, optimalPatchSize());

    if (!m_d->filteredSourceValid) {
        // TODO: make this conversion concurrent!!!
        KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsAlpha(m_d->src);
        filteredMainDev->setDefaultBounds(m_d->src->defaultBounds());

        struct PrefilterSharedState {
            QRect boundingRect;
            KisPaintDeviceSP filteredMainDev;
            KisPaintDeviceSP filteredMainDevSavedCopy;
            QScopedPointer<KisTransaction> activeTransaction;
            FilteringOptions filteringOptions;
        };

        QSharedPointer<PrefilterSharedState> state(new PrefilterSharedState());
        state->boundingRect = m_d->boundingRect;
        state->filteredMainDev = filteredMainDev;
        state->filteringOptions = m_d->filteringOptions;

        if (m_d->filteringOptions.useEdgeDetection &&
            m_d->filteringOptions.edgeDetectionSize > 0.0) {

            addJobSequential(jobs, [state] () {
                state->activeTransaction.reset(new KisTransaction(state->filteredMainDev));
            });

            Q_FOREACH (const QRect &rc, patchRects) {
                addJobConcurrent(jobs, [state, rc] () {
                    KisLodTransformScalar t(state->filteredMainDev);
                    KisGaussianKernel::applyLoG(state->filteredMainDev,
                                                rc,
                                                t.scale(0.5 * state->filteringOptions.edgeDetectionSize),
                                                -1.0,
                                                QBitArray(), 0);
                });
            }

            addJobSequential(jobs, [state] () {
                state->activeTransaction.reset();
                normalizeAlpha8Device(state->filteredMainDev, state->boundingRect);
                state->activeTransaction.reset(new KisTransaction(state->filteredMainDev));
            });

            Q_FOREACH (const QRect &rc, patchRects) {
                addJobConcurrent(jobs, [state, rc] () {
                    KisLodTransformScalar t(state->filteredMainDev);
                    KisGaussianKernel::applyGaussian(state->filteredMainDev,
                                                     rc,
                                                     t.scale(state->filteringOptions.edgeDetectionSize),
                                                     t.scale(state->filteringOptions.edgeDetectionSize),
                                                     QBitArray(), 0);
                });
            }

            addJobSequential(jobs, [state] () {
                state->activeTransaction.reset();
            });
        }

        if (m_d->filteringOptions.fuzzyRadius > 0) {

            addJobSequential(jobs, [state] () {
                state->filteredMainDevSavedCopy = new KisPaintDevice(*state->filteredMainDev);
                state->activeTransaction.reset(new KisTransaction(state->filteredMainDev));
            });

            Q_FOREACH (const QRect &rc, patchRects) {
                addJobConcurrent(jobs, [state, rc] () {
                    KisLodTransformScalar t(state->filteredMainDev);
                    KisGaussianKernel::applyGaussian(state->filteredMainDev,
                                                     rc,
                                                     t.scale(state->filteringOptions.fuzzyRadius),
                                                     t.scale(state->filteringOptions.fuzzyRadius),
                                                     QBitArray(), 0);
                    KisPainter gc(state->filteredMainDev);
                    gc.bitBlt(rc.topLeft(), state->filteredMainDevSavedCopy, rc);
                });
            }

            addJobSequential(jobs, [state] () {
                state->activeTransaction.reset();
            });
        }

        addJobSequential(jobs, [this, state] () {
            normalizeAndInvertAlpha8Device(state->filteredMainDev, state->boundingRect);

            KisDefaultBoundsBaseSP oldBounds = m_d->filteredSource->defaultBounds();
            m_d->filteredSource->makeCloneFrom(state->filteredMainDev, m_d->boundingRect);
            m_d->filteredSource->setDefaultBounds(oldBounds);
            m_d->filteredSourceValid = true;
        });
    }

    if (!m_d->prefilterOnly) {
        addJobSequential(jobs, [this] () {
            m_d->heightMap = new KisPaintDevice(*m_d->filteredSource);
        });

        Q_FOREACH (const QRect &rc, patchRects) {
            addJobConcurrent(jobs, [this, rc] () {
                KritaUtils::filterAlpha8Device(m_d->heightMap, rc,
                                               [](quint8 pixel) {
                                                   return quint8(255 - pixel);
                                               });
            });
        }

        addJobSequential(jobs, [this] () {
            KisProcessingVisitor::ProgressHelper helper(m_d->progressNode);

            KisWatershedWorker worker(m_d->heightMap, m_d->dst, m_d->boundingRect, helper.updater());
            Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
                KoColor color =
                    !stroke.isTransparent ?
                        stroke.color : KoColor(Qt::transparent, m_d->dst->colorSpace());

                worker.addKeyStroke(stroke.dev, color);
            }
            worker.run(m_d->filteringOptions.cleanUpAmount);
        });
    }

    addJobSequential(jobs, [this] () {
        emit sigFinished(m_d->prefilterOnly);
    });

    runnableJobsInterface()->addRunnableJobs(jobs);
}

void KisColorizeStrokeStrategy::cancelStrokeCallback()
{
    emit sigCancelled();
}

KisStrokeStrategy* KisColorizeStrokeStrategy::createLodClone(int levelOfDetail)
{
    KisImageConfig cfg(true);
    if (!cfg.useLodForColorizeMask()) return 0;

    KisColorizeStrokeStrategy *clone = new KisColorizeStrokeStrategy(*this, levelOfDetail);
    return clone;
}
