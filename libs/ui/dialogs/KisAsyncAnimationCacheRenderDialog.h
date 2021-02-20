/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCANIMATIONCACHERENDERDIALOG_H
#define KISASYNCANIMATIONCACHERENDERDIALOG_H

#include "KisAsyncAnimationRenderDialogBase.h"
#include "kis_types.h"


class KisAsyncAnimationCacheRenderDialog : public KisAsyncAnimationRenderDialogBase
{
public:
    KisAsyncAnimationCacheRenderDialog(KisAnimationFrameCacheSP cache, const KisTimeSpan &range, int busyWait = 200);
    virtual ~KisAsyncAnimationCacheRenderDialog();

    static int calcFirstDirtyFrame(KisAnimationFrameCacheSP cache, const KisTimeSpan &playbackRange, const KisTimeSpan &skipRange);

protected:
    QList<int> calcDirtyFrames() const override;
    KisAsyncAnimationRendererBase* createRenderer(KisImageSP image) override;
    void initializeRendererForFrame(KisAsyncAnimationRendererBase *renderer,
                                    KisImageSP image, int frame) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISASYNCANIMATIONCACHERENDERDIALOG_H
