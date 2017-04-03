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

#include "KisAnimationCacheRegenerator.h"

#include "kis_assert.h"
#include <QtConcurrent>
#include <QTimer>
#include <functional>

#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_animation_frame_cache.h"
#include "kis_update_info.h"
#include "kis_signal_auto_connection.h"


struct Q_DECL_HIDDEN KisAnimationCacheRegenerator::Private
{
    int requestedFrame;
    KisAnimationFrameCacheSP requestCache;
    KisOpenGLUpdateInfoSP requestInfo;
    KisSignalAutoConnectionsStore imageRequestConnections;
    QTimer regenerationTimeout;

    QFutureWatcher<void> infoConversionWatcher;

    static const int WAITING_FOR_FRAME_TIMEOUT = 10000;
};

KisAnimationCacheRegenerator::KisAnimationCacheRegenerator(QObject *parent)
    : QObject(parent),
      m_d(new Private)
{
    connect(&m_d->regenerationTimeout, SIGNAL(timeout()), SLOT(slotFrameRegenerationCancelled()));
    connect(this, SIGNAL(sigInternalStartFrameConversion()), SLOT(slotFrameStartConversion()));
    connect(&m_d->infoConversionWatcher, SIGNAL(finished()), SLOT(slotFrameConverted()));

    m_d->regenerationTimeout.setSingleShot(true);
    m_d->regenerationTimeout.setInterval(Private::WAITING_FOR_FRAME_TIMEOUT);
}

KisAnimationCacheRegenerator::~KisAnimationCacheRegenerator()
{
}

void KisAnimationCacheRegenerator::startFrameRegeneration(int frame, KisAnimationFrameCacheSP cache)
{
    KIS_ASSERT_RECOVER_NOOP(QThread::currentThread() == this->thread());

    KisImageSP image = cache->image();

    m_d->requestCache = cache;
    m_d->requestedFrame = frame;

    m_d->imageRequestConnections.clear();
    m_d->imageRequestConnections.addConnection(
                image->animationInterface(), SIGNAL(sigFrameReady(int)),
                this, SLOT(slotFrameRegenerationFinished(int)),
                Qt::DirectConnection);

    m_d->imageRequestConnections.addConnection(
                image->animationInterface(), SIGNAL(sigFrameCancelled()),
                this, SLOT(slotFrameRegenerationCancelled()),
                Qt::AutoConnection);

    m_d->regenerationTimeout.start();
    image->animationInterface()->requestFrameRegeneration(frame, image->bounds());
}

void KisAnimationCacheRegenerator::cancelCurrentFrameRegeneration()
{
    m_d->imageRequestConnections.clear();
    m_d->requestCache = 0;
    m_d->requestedFrame = -1;
    m_d->requestInfo = 0;
    m_d->regenerationTimeout.stop();
}

void KisAnimationCacheRegenerator::slotFrameRegenerationCancelled()
{
    // the timeout can arrive in async way
    if (!m_d->requestCache) return;

    cancelCurrentFrameRegeneration();
    emit sigFrameCancelled();
}

void KisAnimationCacheRegenerator::slotFrameRegenerationFinished(int frame)
{
    // WARNING: executed in the context of image worker thread!

    KisAnimationFrameCacheSP cache = m_d->requestCache;
    if (!cache) return;

    // probably a bit too strict...
    KIS_SAFE_ASSERT_RECOVER_RETURN(frame == m_d->requestedFrame);

    m_d->imageRequestConnections.clear();
    m_d->requestInfo = cache->fetchFrameData(frame);

    emit sigInternalStartFrameConversion();
}

namespace {
static void processFrameInfo(KisOpenGLUpdateInfoSP info)
{
    if (info->needsConversion()) {
        info->convertColorSpace();
    }
}
}

void KisAnimationCacheRegenerator::slotFrameStartConversion()
{
    if (!m_d->requestInfo) return;

    m_d->regenerationTimeout.stop();

    QFuture<void> requestFuture =
        QtConcurrent::run(
            std::bind(&processFrameInfo, m_d->requestInfo));

    m_d->infoConversionWatcher.setFuture(requestFuture);
}

void KisAnimationCacheRegenerator::slotFrameConverted()
{
    if (!m_d->requestInfo || !m_d->requestCache) return;

    m_d->requestCache->addConvertedFrameData(m_d->requestInfo, m_d->requestedFrame);

    m_d->requestCache = 0;
    m_d->requestedFrame = -1;
    m_d->requestInfo = 0;

    emit sigFrameFinished();
}

