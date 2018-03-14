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

#ifndef KISASYNCANIMATIONCACHERENDERDIALOG_H
#define KISASYNCANIMATIONCACHERENDERDIALOG_H

#include "KisAsyncAnimationRenderDialogBase.h"
#include "kis_types.h"


class KisAsyncAnimationCacheRenderDialog : public KisAsyncAnimationRenderDialogBase
{
public:
    KisAsyncAnimationCacheRenderDialog(KisAnimationFrameCacheSP cache, const KisTimeRange &range, int busyWait = 200);
    ~KisAsyncAnimationCacheRenderDialog();

    static int calcFirstDirtyFrame(KisAnimationFrameCacheSP cache, const KisTimeRange &playbackRange, const KisTimeRange &skipRange);

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
