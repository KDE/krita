/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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

#ifndef KISASYNCSTORYBOARDTHUMBNAILRENDERER_H
#define KISASYNCSTORYBOARDTHUMBNAILRENDERER_H

#include <KisAsyncAnimationRendererBase.h>

class KisPaintDevice;
class KisAsyncStoryboardThumbnailRenderer : public KisAsyncAnimationRendererBase
{
    Q_OBJECT
public:
    KisAsyncStoryboardThumbnailRenderer();
    ~KisAsyncStoryboardThumbnailRenderer();

    KisPaintDeviceSP frameProjection()
    {
        return m_requestedFrameProjection;
    }

protected:
    void frameCompletedCallback(int frame, const KisRegion &requestedRegion) override;
    void frameCancelledCallback(int frame) override;
    void clearFrameRegenerationState(bool isCancelled) override;

private:
    KisPaintDeviceSP m_requestedFrameProjection;
};

#endif
