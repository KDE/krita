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
    void frameCompletedCallback(int frame, const QRegion &requestedRegion) override;
    void frameCancelledCallback(int frame) override;
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
