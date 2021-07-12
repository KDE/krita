/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCANIMATIONCACHERENDERER_H
#define KISASYNCANIMATIONCACHERENDERER_H

#include <KisAsyncAnimationRendererBase.h>

class KisAsyncAnimationCacheRenderer : public KisAsyncAnimationRendererBase
{
    Q_OBJECT
public:
    KisAsyncAnimationCacheRenderer();
    ~KisAsyncAnimationCacheRenderer();

    void setFrameCache(KisAnimationFrameCacheSP cache);

protected:
    void frameCompletedCallback(int frame, const KisRegion &requestedRegion) override;
    void frameCancelledCallback(int frame, CancelReason cancelReason) override;
    void clearFrameRegenerationState(bool isCancelled) override;


Q_SIGNALS:
    void sigCompleteRegenerationInternal(int frame);

private Q_SLOTS:
    void slotCompleteRegenerationInternal(int frame);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISASYNCANIMATIONCACHERENDERER_H
