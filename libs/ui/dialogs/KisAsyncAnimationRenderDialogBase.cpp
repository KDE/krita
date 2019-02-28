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

#include "KisAsyncAnimationRenderDialogBase.h"

#include <QEventLoop>
#include <QProgressDialog>
#include <QElapsedTimer>
#include <QApplication>
#include <QThread>
#include <QTime>
#include <QList>
#include <QtMath>

#include <klocalizedstring.h>

#include "KisViewManager.h"
#include "KisAsyncAnimationRendererBase.h"
#include "kis_time_range.h"
#include "kis_image.h"
#include "kis_image_config.h"
#include "kis_memory_statistics_server.h"
#include "kis_signal_compressor.h"
#include <boost/optional.hpp>

#include <vector>
#include <memory>

namespace {
struct RendererPair {
    std::unique_ptr<KisAsyncAnimationRendererBase> renderer;
    KisImageSP image;

    RendererPair() {}
    RendererPair(KisAsyncAnimationRendererBase *_renderer, KisImageSP _image)
        : renderer(_renderer),
          image(_image)
    {
    }
    RendererPair(RendererPair &&rhs)
        : renderer(std::move(rhs.renderer)),
          image(rhs.image)
    {
    }
};

int calculateNumberMemoryAllowedClones(KisImageSP image)
{
    KisMemoryStatisticsServer::Statistics stats =
        KisMemoryStatisticsServer::instance()
        ->fetchMemoryStatistics(image);

    const qint64 allowedMemory = 0.8 * stats.tilesHardLimit - stats.realMemorySize;
    const qint64 cloneSize = stats.projectionsSize;

    return cloneSize > 0 ? allowedMemory / cloneSize : 0;
}

}


struct KisAsyncAnimationRenderDialogBase::Private
{
    Private(const QString &_actionTitle, KisImageSP _image, int _busyWait)
        : actionTitle(_actionTitle),
          image(_image),
          busyWait(_busyWait),
          progressDialogCompressor(40, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    QString actionTitle;
    KisImageSP image;
    int busyWait;
    bool isBatchMode = false;

    std::vector<RendererPair> asyncRenderers;
    bool memoryLimitReached = false;

    QElapsedTimer processingTime;
    QScopedPointer<QProgressDialog> progressDialog;
    QEventLoop waitLoop;

    QList<int> stillDirtyFrames;
    QList<int> framesInProgress;
    int dirtyFramesCount = 0;
    Result result = RenderComplete;
    QRegion regionOfInterest;

    KisSignalCompressor progressDialogCompressor;
    using ProgressData = QPair<int, QString>;
    boost::optional<ProgressData> progressData;
    int progressDialogReentrancyCounter = 0;


    int numDirtyFramesLeft() const {
        return stillDirtyFrames.size() + framesInProgress.size();
    }

};

KisAsyncAnimationRenderDialogBase::KisAsyncAnimationRenderDialogBase(const QString &actionTitle, KisImageSP image, int busyWait)
    : m_d(new Private(actionTitle, image, busyWait))
{
    connect(&m_d->progressDialogCompressor, SIGNAL(timeout()),
            SLOT(slotUpdateCompressedProgressData()), Qt::QueuedConnection);
}

KisAsyncAnimationRenderDialogBase::~KisAsyncAnimationRenderDialogBase()
{
}

KisAsyncAnimationRenderDialogBase::Result
KisAsyncAnimationRenderDialogBase::regenerateRange(KisViewManager *viewManager)
{
    {
        /**
         * Since this method can be called from the places where no
         * view manager is available, we need this manually crafted
         * ugly construction to "try-lock-cancel" the image.
         */

        bool imageIsIdle = true;

        if (viewManager) {
            imageIsIdle = viewManager->blockUntilOperationsFinished(m_d->image);
        } else {
            imageIsIdle = false;
            if (m_d->image->tryBarrierLock(true)) {
                m_d->image->unlock();
                imageIsIdle = true;
            }
        }

        if (!imageIsIdle) {
            return RenderCancelled;
        }
    }

    m_d->stillDirtyFrames = calcDirtyFrames();
    m_d->framesInProgress.clear();
    m_d->result = RenderComplete;
    m_d->dirtyFramesCount = m_d->stillDirtyFrames.size();

    if (!m_d->isBatchMode) {
        QWidget *parentWidget = viewManager ? viewManager->mainWindow() : 0;
        m_d->progressDialog.reset(new QProgressDialog(m_d->actionTitle, i18n("Cancel"), 0, 0, parentWidget));
        m_d->progressDialog->setWindowModality(Qt::ApplicationModal);
        m_d->progressDialog->setMinimum(0);
        m_d->progressDialog->setMaximum(m_d->dirtyFramesCount);
        m_d->progressDialog->setMinimumDuration(m_d->busyWait);
        connect(m_d->progressDialog.data(), SIGNAL(canceled()), SLOT(slotCancelRegeneration()));
    }

    if (m_d->dirtyFramesCount <= 0) return m_d->result;

    m_d->processingTime.start();

    KisImageConfig cfg(true);

    const int maxThreads = cfg.maxNumberOfThreads();
    const int numAllowedWorker = 1 + calculateNumberMemoryAllowedClones(m_d->image);
    const int proposedNumWorkers = qMin(m_d->dirtyFramesCount, cfg.frameRenderingClones());
    const int numWorkers = qMin(proposedNumWorkers, numAllowedWorker);
    const int numThreadsPerWorker = qMax(1, qCeil(qreal(maxThreads) / numWorkers));

    m_d->memoryLimitReached = numWorkers < proposedNumWorkers;

    const int oldWorkingThreadsLimit = m_d->image->workingThreadsLimit();

    ENTER_FUNCTION() << ppVar(numWorkers) << ppVar(numThreadsPerWorker);

    for (int i = 0; i < numWorkers; i++) {
        // reuse the image for one of the workers
        KisImageSP image = i == numWorkers - 1 ? m_d->image : m_d->image->clone(true);

        image->setWorkingThreadsLimit(numThreadsPerWorker);
        KisAsyncAnimationRendererBase *renderer = createRenderer(image);

        connect(renderer, SIGNAL(sigFrameCompleted(int)), SLOT(slotFrameCompleted(int)));
        connect(renderer, SIGNAL(sigFrameCancelled(int)), SLOT(slotFrameCancelled(int)));

        m_d->asyncRenderers.push_back(RendererPair(renderer, image));
    }

    ENTER_FUNCTION() << "Copying done in" << m_d->processingTime.elapsed();

    tryInitiateFrameRegeneration();
    updateProgressLabel();

    if (m_d->numDirtyFramesLeft() > 0) {
        m_d->waitLoop.exec();
    }

    ENTER_FUNCTION() << "Full regeneration done in" << m_d->processingTime.elapsed();

    for (auto &pair : m_d->asyncRenderers) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!pair.renderer->isActive());
        if (viewManager) {
            viewManager->blockUntilOperationsFinishedForced(pair.image);
        } else {
            pair.image->barrierLock(true);
            pair.image->unlock();
        }

    }
    m_d->asyncRenderers.clear();

    if (viewManager) {
        viewManager->blockUntilOperationsFinishedForced(m_d->image);
    } else {
        m_d->image->barrierLock(true);
        m_d->image->unlock();
    }

    m_d->image->setWorkingThreadsLimit(oldWorkingThreadsLimit);

    m_d->progressDialog.reset();

    return m_d->result;
}

void KisAsyncAnimationRenderDialogBase::setRegionOfInterest(const QRegion &roi)
{
    m_d->regionOfInterest = roi;
}

QRegion KisAsyncAnimationRenderDialogBase::regionOfInterest() const
{
    return m_d->regionOfInterest;
}

void KisAsyncAnimationRenderDialogBase::slotFrameCompleted(int frame)
{
    Q_UNUSED(frame);

    m_d->framesInProgress.removeOne(frame);

    tryInitiateFrameRegeneration();
    updateProgressLabel();
}

void KisAsyncAnimationRenderDialogBase::slotFrameCancelled(int frame)
{
    Q_UNUSED(frame);

    cancelProcessingImpl(false);
}

void KisAsyncAnimationRenderDialogBase::slotCancelRegeneration()
{
    cancelProcessingImpl(true);
}

void KisAsyncAnimationRenderDialogBase::cancelProcessingImpl(bool isUserCancelled)
{
    for (auto &pair : m_d->asyncRenderers) {
        if (pair.renderer->isActive()) {
            pair.renderer->cancelCurrentFrameRendering();
        }
        KIS_SAFE_ASSERT_RECOVER_NOOP(!pair.renderer->isActive());
    }

    m_d->stillDirtyFrames.clear();
    m_d->framesInProgress.clear();
    m_d->result = isUserCancelled ? RenderCancelled : RenderFailed;
    updateProgressLabel();
}


void KisAsyncAnimationRenderDialogBase::tryInitiateFrameRegeneration()
{
    bool hadWorkOnPreviousCycle = false;

    while (!m_d->stillDirtyFrames.isEmpty()) {
        for (auto &pair : m_d->asyncRenderers) {
            if (!pair.renderer->isActive()) {
                const int currentDirtyFrame = m_d->stillDirtyFrames.takeFirst();

                initializeRendererForFrame(pair.renderer.get(), pair.image, currentDirtyFrame);
                pair.renderer->startFrameRegeneration(pair.image, currentDirtyFrame, m_d->regionOfInterest);
                hadWorkOnPreviousCycle = true;
                m_d->framesInProgress.append(currentDirtyFrame);
                break;
            }
        }

        if (!hadWorkOnPreviousCycle) break;
        hadWorkOnPreviousCycle = false;
    }
}

void KisAsyncAnimationRenderDialogBase::updateProgressLabel()
{
    const int processedFramesCount = m_d->dirtyFramesCount - m_d->numDirtyFramesLeft();

    const qint64 elapsedMSec = m_d->processingTime.elapsed();
    const qint64 estimatedMSec =
        !processedFramesCount ? 0 :
        elapsedMSec * m_d->dirtyFramesCount / processedFramesCount;

    const QTime elapsedTime = QTime::fromMSecsSinceStartOfDay(elapsedMSec);
    const QTime estimatedTime = QTime::fromMSecsSinceStartOfDay(estimatedMSec);

    const QString timeFormat = estimatedTime.hour() > 0 ? "HH:mm:ss" : "mm:ss";

    const QString elapsedTimeString = elapsedTime.toString(timeFormat);
    const QString estimatedTimeString = estimatedTime.toString(timeFormat);

    const QString memoryLimitMessage(
        i18n("\n\nMemory limit is reached!\nThe number of clones is limited to %1\n\n",
             m_d->asyncRenderers.size()));


    const QString progressLabel(i18n("%1\n\nElapsed: %2\nEstimated: %3\n\n%4",
                                     m_d->actionTitle,
                                     elapsedTimeString,
                                     estimatedTimeString,
                                     m_d->memoryLimitReached ? memoryLimitMessage : QString()));
    if (m_d->progressDialog) {
        /**
         * We should avoid reentrancy caused by explicit
         * QApplication::processEvents() in QProgressDialog::setValue(), so use
         * a compressor instead
         */
        m_d->progressData = Private::ProgressData(processedFramesCount, progressLabel);
        m_d->progressDialogCompressor.start();
    }

    if (!m_d->numDirtyFramesLeft()) {
        m_d->waitLoop.quit();
    }
}

void KisAsyncAnimationRenderDialogBase::slotUpdateCompressedProgressData()
{
    /**
     * Qt's implementation of QProgressDialog is a bit weird: it calls
     * QApplication::processEvents() from inside setValue(), which means
     * that our update method may reenter multiple times.
     *
     * This code avoids reentering by using a compresson and an explicit
     * entrance counter.
     */

    if (m_d->progressDialogReentrancyCounter > 0) {
        m_d->progressDialogCompressor.start();
        return;
    }

    if (m_d->progressDialog && m_d->progressData) {
        m_d->progressDialogReentrancyCounter++;

        m_d->progressDialog->setLabelText(m_d->progressData->second);
        m_d->progressDialog->setValue(m_d->progressData->first);
        m_d->progressData = boost::none;

        m_d->progressDialogReentrancyCounter--;
    }
}

void KisAsyncAnimationRenderDialogBase::setBatchMode(bool value)
{
    m_d->isBatchMode = value;
}

bool KisAsyncAnimationRenderDialogBase::batchMode() const
{
    return m_d->isBatchMode;
}
