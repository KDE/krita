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
#include <KisAnimationCacheRegenerator.h>
#include "kis_animation_frame_cache.h"
#include "kis_time_range.h"
#include "kis_image.h"
#include "KisViewManager.h"

struct KisAnimationCacheUpdateProgressDialog::Private
{
    Private(int _busyWait, QWidget *parent)
        : busyWait(_busyWait),
          progressDialog(i18n("Regenerating cache..."), i18n("Cancel"), 0, 0, parent)
    {
        progressDialog.setWindowModality(Qt::ApplicationModal);
        connect(&progressDialog, SIGNAL(canceled()), &regenerator, SLOT(cancelCurrentFrameRegeneration()));
    }

    int busyWait;
    KisAnimationCacheRegenerator regenerator;

    KisAnimationFrameCacheSP cache;
    KisTimeRange playbackRange;
    int dirtyFramesCount = 0;
    int processedFramesCount = 0;
    bool hasSomethingToDo = true;

    QProgressDialog progressDialog;
};

KisAnimationCacheUpdateProgressDialog::KisAnimationCacheUpdateProgressDialog(int busyWait, QWidget *parent)
    : QObject(parent),
      m_d(new Private(busyWait, parent))
{
    connect(&m_d->regenerator, SIGNAL(sigFrameFinished()), SLOT(slotFrameFinished()));
    connect(&m_d->regenerator, SIGNAL(sigFrameCancelled()), SLOT(slotFrameCancelled()));
}

KisAnimationCacheUpdateProgressDialog::~KisAnimationCacheUpdateProgressDialog()
{
}

void KisAnimationCacheUpdateProgressDialog::regenerateRange(KisAnimationFrameCacheSP cache, const KisTimeRange &playbackRange, KisViewManager *viewManager)
{
    m_d->cache = cache;
    m_d->playbackRange = playbackRange;

    m_d->dirtyFramesCount = m_d->regenerator.calcNumberOfDirtyFrame(m_d->cache, m_d->playbackRange);

    m_d->progressDialog.setMaximum(m_d->dirtyFramesCount);

    // HACK ALERT: since the slot is named 'finished', so it increments
    //             the preseccedFramesCount field on every call. And since
    //             this is a cold-start, we should decrement it in advance.
    m_d->processedFramesCount = -1;
    slotFrameFinished();

    QElapsedTimer t;
    t.start();

    while (t.elapsed() < m_d->busyWait) {
        QApplication::processEvents();

        if (!m_d->hasSomethingToDo) {
            break;
        }

        QThread::yieldCurrentThread();
    }


    if (m_d->hasSomethingToDo) {
        m_d->progressDialog.exec();
    }

    KisImageSP image = cache->image();
    viewManager->blockUntilOperationsFinishedForced(image);
}

void KisAnimationCacheUpdateProgressDialog::slotFrameFinished()
{
    m_d->processedFramesCount++;
    int currentDirtyFrame = m_d->regenerator.calcFirstDirtyFrame(m_d->cache, m_d->playbackRange, KisTimeRange());

    if (currentDirtyFrame >= 0) {
        m_d->regenerator.startFrameRegeneration(currentDirtyFrame, m_d->cache);
    } else {
        m_d->hasSomethingToDo = false;
        m_d->processedFramesCount = m_d->dirtyFramesCount;
    }

    m_d->progressDialog.setValue(m_d->processedFramesCount);
}

void KisAnimationCacheUpdateProgressDialog::slotFrameCancelled()
{
    m_d->hasSomethingToDo = false;
    m_d->processedFramesCount = m_d->dirtyFramesCount;
    m_d->progressDialog.setValue(m_d->processedFramesCount);
}

