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

#include "KisAnimationCacheUpdateProgressDialog.h"

#include <QEventLoop>
#include <QProgressDialog>
#include <QElapsedTimer>
#include <QApplication>
#include <QThread>
#include <QTime>
#include <KisAnimationCacheRegenerator.h>
#include "kis_animation_frame_cache.h"
#include "kis_time_range.h"
#include "kis_image.h"
#include "KisViewManager.h"
#include <QThread>
#include <memory>

struct KisAnimationCacheUpdateProgressDialog::Private
{
    Private(int _busyWait, QWidget *parent)
        : busyWait(_busyWait),
          progressDialog(i18n("Regenerating cache..."), i18n("Cancel"), 0, 0, parent)
    {
        progressDialog.setWindowModality(Qt::ApplicationModal);
    }

    int busyWait;

    struct AsyncRegenerationPair {
        AsyncRegenerationPair(KisImageSP _image) : image(_image) {}
        KisImageSP image;
        KisAnimationCacheRegenerator regenerator;
    };

    typedef std::unique_ptr<AsyncRegenerationPair> AsyncRegenerationPairUP;

    std::vector<AsyncRegenerationPairUP> asyncRegenerators;


    KisAnimationFrameCacheSP cache;
    KisTimeRange playbackRange;
    int dirtyFramesCount = 0;
    QList<int> stillDirtyFrames;
    int processedFramesCount = 0;
    bool hasSomethingToDo = true;
    QElapsedTimer processingTime;

    QProgressDialog progressDialog;
};

KisAnimationCacheUpdateProgressDialog::KisAnimationCacheUpdateProgressDialog(int busyWait, QWidget *parent)
    : QObject(parent),
      m_d(new Private(busyWait, parent))
{
}

KisAnimationCacheUpdateProgressDialog::~KisAnimationCacheUpdateProgressDialog()
{
}

void KisAnimationCacheUpdateProgressDialog::regenerateRange(KisAnimationFrameCacheSP cache, const KisTimeRange &playbackRange, KisViewManager *viewManager)
{
    m_d->cache = cache;
    m_d->playbackRange = playbackRange;

    m_d->stillDirtyFrames = KisAnimationCacheRegenerator::calcDirtyFramesList(m_d->cache, m_d->playbackRange);
    m_d->dirtyFramesCount = m_d->stillDirtyFrames.size();
    m_d->progressDialog.setMaximum(m_d->dirtyFramesCount);

    if (m_d->dirtyFramesCount <= 0) return;

    m_d->processingTime.start();

    for (int i = 0; i < QThread::idealThreadCount(); i++) {

        Private::AsyncRegenerationPairUP pair(
            new Private::AsyncRegenerationPair(m_d->cache->image()->clone(true)));

        KisAnimationCacheRegenerator &regenerator = pair->regenerator;

        connect(&m_d->progressDialog, SIGNAL(canceled()), &regenerator, SLOT(cancelCurrentFrameRegeneration()));
        connect(&regenerator, SIGNAL(sigFrameFinished()), SLOT(slotFrameFinished()));
        connect(&regenerator, SIGNAL(sigFrameCancelled()), SLOT(slotFrameCancelled()));

        m_d->asyncRegenerators.push_back(std::move(pair));
    }

    ENTER_FUNCTION() << "Copying done in" << m_d->processingTime.elapsed();

    tryInitiateFrameRegeneration();
    updateProgressLabel();

    while (m_d->processingTime.elapsed() < m_d->busyWait) {
        QApplication::processEvents();

        if (!m_d->hasSomethingToDo) {
            break;
        }

        QThread::yieldCurrentThread();
    }


    if (m_d->hasSomethingToDo) {
        m_d->progressDialog.exec();
    }

    ENTER_FUNCTION() << "Full regeneration done in" << m_d->processingTime.elapsed();

    for (auto &pair : m_d->asyncRegenerators) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!pair->regenerator.isActive());
        viewManager->blockUntilOperationsFinishedForced(pair->image);
    }
    m_d->asyncRegenerators.clear();

    KisImageSP image = cache->image();
    viewManager->blockUntilOperationsFinishedForced(image);
}

void KisAnimationCacheUpdateProgressDialog::tryInitiateFrameRegeneration()
{
    bool hadWorkOnPreviousCycle = false;

    while (!m_d->stillDirtyFrames.isEmpty()) {
        for (auto &pair : m_d->asyncRegenerators) {
            if (!pair->regenerator.isActive()) {
                const int currentDirtyFrame = m_d->stillDirtyFrames.takeFirst();
                pair->regenerator.startFrameRegeneration(currentDirtyFrame, pair->image, m_d->cache);
                hadWorkOnPreviousCycle = true;
                break;
            }
        }

        if (!hadWorkOnPreviousCycle) break;
        hadWorkOnPreviousCycle = false;
    }
}

void KisAnimationCacheUpdateProgressDialog::updateProgressLabel()
{
    const qint64 elapsedMSec = m_d->processingTime.elapsed();
    const qint64 estimatedMSec =
        !m_d->processedFramesCount ? 0 :
        elapsedMSec * m_d->dirtyFramesCount / m_d->processedFramesCount;

    const QTime elapsedTime = QTime::fromMSecsSinceStartOfDay(elapsedMSec);
    const QTime estimatedTime = QTime::fromMSecsSinceStartOfDay(estimatedMSec);

    const QString timeFormat = estimatedTime.hour() > 0 ? "HH:mm:ss" : "mm:ss";

    const QString elapsedTimeString = elapsedTime.toString(timeFormat);
    const QString estimatedTimeString = estimatedTime.toString(timeFormat);

    const QString progressLabel(i18n("Regenerating cache:\n\nElapsed: %1\nEstimated: %2\n\n",
                                         elapsedTimeString, estimatedTimeString));
    m_d->progressDialog.setLabelText(progressLabel);
}


void KisAnimationCacheUpdateProgressDialog::slotFrameFinished()
{
    m_d->processedFramesCount++;

    // check we don't bite more than we can chew
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->processedFramesCount <= m_d->dirtyFramesCount);

    if (m_d->processedFramesCount < m_d->dirtyFramesCount) {
        tryInitiateFrameRegeneration();
    } else {
        m_d->hasSomethingToDo = false;
        m_d->processedFramesCount = m_d->dirtyFramesCount;
    }

    m_d->progressDialog.setValue(m_d->processedFramesCount);
    updateProgressLabel();
}

void KisAnimationCacheUpdateProgressDialog::slotFrameCancelled()
{
    for (auto &pair : m_d->asyncRegenerators) {
        if (pair->regenerator.isActive()) {
            pair->regenerator.cancelCurrentFrameRegeneration();
        }
        KIS_SAFE_ASSERT_RECOVER_NOOP(!pair->regenerator.isActive());
    }

    m_d->hasSomethingToDo = false;
    m_d->processedFramesCount = m_d->dirtyFramesCount;
    m_d->progressDialog.setValue(m_d->processedFramesCount);
}

