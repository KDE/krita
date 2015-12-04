/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_animation_cache_populator.h"

#include <functional>

#include <QTimer>
#include <QMutex>
#include <QtConcurrent>

#include "KisPart.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_canvas2.h"
#include "kis_time_range.h"
#include "kis_animation_frame_cache.h"
#include "kis_update_info.h"
#include "kis_signal_auto_connection.h"
#include "kis_idle_watcher.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_keyframe_channel.h"


struct KisAnimationCachePopulator::Private
{
    KisAnimationCachePopulator *q;
    KisPart *part;

    QTimer timer;

    /**
     * Counts up the number of subsequent times Krita has been detected idle.
     */
    int idleCounter;
    static const int IDLE_COUNT_THRESHOLD = 4;
    static const int IDLE_CHECK_INTERVAL = 500;
    static const int WAITING_FOR_FRAME_TIMEOUT = 3000;
    static const int BETWEEN_FRAMES_INTERVAL = 10;

    int requestedFrame;
    KisAnimationFrameCacheSP requestCache;
    KisOpenGLUpdateInfoSP requestInfo;
    QScopedPointer<KisSignalAutoConnection> imageRequestConnection;

    QFutureWatcher<void> infoConversionWatcher;



    enum State {
        NotWaitingForAnything,
        WaitingForIdle,
        WaitingForFrame,
        WaitingForConvertedFrame,
        BetweenFrames
    };
    State state;

    QMutex mutex;

    Private(KisAnimationCachePopulator *_q, KisPart *_part)
        : q(_q),
          part(_part),
          idleCounter(0),
          requestedFrame(-1),
          state(WaitingForIdle)
    {
        timer.setSingleShot(true);
        connect(&infoConversionWatcher, SIGNAL(finished()), q, SLOT(slotInfoConverted()));
    }

    static void processFrameInfo(KisOpenGLUpdateInfoSP info) {
        if (info->needsConversion()) {
            info->convertColorSpace();
        }
    }

    void frameReceived(int frame)
    {
        if (frame != requestedFrame) return;

        imageRequestConnection.reset();
        requestInfo = requestCache->fetchFrameData(frame);

        /**
         * This method is called from the context of the image worker
         * threads, so we cannot modify timers here. Therefore the
         * timers reset and the conversion request will be issued in
         * the main GUI thread.
         */
        emit q->sigPrivateStartWaitingForConvertedFrame();
    }

    void infoConverted() {
        KIS_ASSERT_RECOVER(requestInfo && requestCache) {
            enterState(WaitingForIdle);
            return;
        }

        requestCache->addConvertedFrameData(requestInfo, requestedFrame);

        requestedFrame = 0;
        requestCache = 0;
        requestInfo = 0;
        enterState(BetweenFrames);
    }

    void timerTimeout() {
        switch (state) {
        case WaitingForIdle:
        case BetweenFrames:
            generateIfIdle();
            break;
        case WaitingForFrame:
            // Request timed out :(
            imageRequestConnection.reset();
            enterState(WaitingForIdle);
            break;
        case WaitingForConvertedFrame:
            KIS_ASSERT_RECOVER_NOOP(0 && "WaitingForConvertedFrame cannot have a timeout. Just skip this message and report a bug");
            break;
        case NotWaitingForAnything:
            KIS_ASSERT_RECOVER_NOOP(0 && "NotWaitingForAnything cannot have a timeout. Just skip this message and report a bug");
            break;
        }
    }

    void generateIfIdle()
    {
        if (part->idleWatcher()->isIdle()) {
            idleCounter++;

            if (idleCounter >= IDLE_COUNT_THRESHOLD) {
                if (!tryRequestGeneration()) {
                    enterState(NotWaitingForAnything);
                }
                return;
            }
        } else {
            idleCounter = 0;
        }

        enterState(WaitingForIdle);
    }


    bool tryRequestGeneration()
    {
        // Prioritize the active document
        KisAnimationFrameCacheSP activeDocumentCache = KisAnimationFrameCacheSP(0);

        KisMainWindow *activeWindow = part->currentMainwindow();
        if (activeWindow && activeWindow->activeView()) {
            KisCanvas2 *activeCanvas = activeWindow->activeView()->canvasBase();

            if (activeCanvas && activeCanvas->frameCache()) {
                activeDocumentCache = activeCanvas->frameCache();

                // Let's skip frames affected by changes to the active node (on the active document)
                // This avoids constant invalidation and regeneration while drawing
                KisNodeSP activeNode = activeCanvas->viewManager()->nodeManager()->activeNode();
                KisTimeRange skipRange;
                if (activeNode) {
                    int currentTime = activeCanvas->currentImage()->animationInterface()->currentUITime();

                    KisKeyframeChannel *channel;
                    Q_FOREACH (channel, activeNode->keyframeChannels()) {
                        skipRange |= channel->affectedFrames(currentTime);
                    }
                }

                bool requested = tryRequestGeneration(activeDocumentCache, skipRange);
                if (requested) return true;
            }
        }

        QList<KisAnimationFrameCache*> caches = KisAnimationFrameCache::caches();
        KisAnimationFrameCache *cache;
        Q_FOREACH (cache, caches) {
            if (cache == activeDocumentCache.data()) {
                // We already handled this one...
                continue;
            }

            bool requested = tryRequestGeneration(cache, KisTimeRange());
            if (requested) return true;
        }

        return false;
    }

    bool tryRequestGeneration(KisAnimationFrameCacheSP cache, KisTimeRange skipRange)
    {
        KisImageSP image = cache->image();
        if (!image) return false;

        KisImageAnimationInterface *animation = image->animationInterface();
        KisTimeRange currentRange = animation->currentRange();

        if (currentRange.isValid()) {
            Q_ASSERT(!currentRange.isInfinite());

            // TODO: optimize check for fully-cached case

            for (int frame = currentRange.start(); frame <= currentRange.end(); frame++) {
                if (skipRange.contains(frame)) {
                    if (skipRange.isInfinite()) {
                        break;
                    } else {
                        frame = skipRange.end();
                        continue;
                    }
                }

                if (!cache->frameStatus(frame) == KisAnimationFrameCache::Cached) {
                    return regenerate(cache, frame);
                }
            }
        }

        return false;
    }

    bool regenerate(KisAnimationFrameCacheSP cache, int frame)
    {
        if (state == WaitingForFrame || state == WaitingForConvertedFrame) {
            // Already busy, deny request
            return false;
        }

        KIS_ASSERT_RECOVER_NOOP(QThread::currentThread() == q->thread());

        KisImageSP image = cache->image();

        requestCache = cache;
        requestedFrame = frame;

        imageRequestConnection.reset(
            new KisSignalAutoConnection(
                image->animationInterface(), SIGNAL(sigFrameReady(int)),
                q, SLOT(slotFrameReady(int)),
                Qt::DirectConnection));

        /**
         * We should enter the state before the frame is
         * requested. Otherwise the signal may come earlier than we
         * enter it.
         */
        enterState(WaitingForFrame);
        image->animationInterface()->requestFrameRegeneration(frame, image->bounds());

        return true;
    }

    QString debugStateToString(State newState) {
        QString str = "<unknown>";

        switch (newState) {
        case WaitingForIdle:
            str = "WaitingForIdle";
            break;
        case WaitingForFrame:
            str = "WaitingForFrame";
            break;
        case NotWaitingForAnything:
            str = "NotWaitingForAnything";
            break;
        case WaitingForConvertedFrame:
            str = "WaitingForConvertedFrame";
            break;
        case BetweenFrames:
            str = "BetweenFrames";
            break;
        }

        return str;
    }

    void enterState(State newState)
    {
        state = newState;
        int timerTimeout = -1;

        switch (state) {
        case WaitingForIdle:
            timerTimeout = IDLE_CHECK_INTERVAL;
            break;
        case WaitingForFrame:
            timerTimeout = WAITING_FOR_FRAME_TIMEOUT;
            break;
        case NotWaitingForAnything:
        case WaitingForConvertedFrame:
            // frame conversion cannot be cancelled,
            // so there is no timeout
            timerTimeout = -1;
            break;
        case BetweenFrames:
            timerTimeout = BETWEEN_FRAMES_INTERVAL;
            break;
        }

        if (timerTimeout >= 0) {
            timer.start(timerTimeout);
        } else {
            timer.stop();
        }
    }
};

KisAnimationCachePopulator::KisAnimationCachePopulator(KisPart *part)
    : m_d(new Private(this, part))
{
    connect(&m_d->timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    connect(this, SIGNAL(sigPrivateStartWaitingForConvertedFrame()), SLOT(slotPrivateStartWaitingForConvertedFrame()));
}

KisAnimationCachePopulator::~KisAnimationCachePopulator()
{}

bool KisAnimationCachePopulator::regenerate(KisAnimationFrameCacheSP cache, int frame)
{
    return m_d->regenerate(cache, frame);
}

void KisAnimationCachePopulator::slotStart()
{
    m_d->timer.start();
}

void KisAnimationCachePopulator::slotTimer()
{
    m_d->timerTimeout();
}

void KisAnimationCachePopulator::slotFrameReady(int frame)
{
    m_d->frameReceived(frame);
}

void KisAnimationCachePopulator::slotInfoConverted()
{
    m_d->infoConverted();
}

void KisAnimationCachePopulator::slotRequestRegeneration()
{
    m_d->enterState(Private::WaitingForIdle);
}

void KisAnimationCachePopulator::slotPrivateStartWaitingForConvertedFrame()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->requestInfo);

    m_d->enterState(Private::WaitingForConvertedFrame);

    QFuture<void> requestFuture =
        QtConcurrent::run(
            std::bind(&KisAnimationCachePopulator::Private::processFrameInfo,
                        m_d->requestInfo));

    m_d->infoConversionWatcher.setFuture(requestFuture);
}

#include "kis_animation_cache_populator.moc"
