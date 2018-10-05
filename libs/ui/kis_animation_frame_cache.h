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

#ifndef KIS_ANIMATION_FRAME_CACHE_H
#define KIS_ANIMATION_FRAME_CACHE_H

#include <QImage>
#include <QObject>

#include "kritaui_export.h"
#include "kis_types.h"
#include "kis_shared.h"

class KisImage;
class KisImageAnimationInterface;
class KisTimeSpan;
class KisFrameSet;

class KisOpenGLImageTextures;
typedef KisSharedPtr<KisOpenGLImageTextures> KisOpenGLImageTexturesSP;

class KisOpenGLUpdateInfo;
typedef KisSharedPtr<KisOpenGLUpdateInfo> KisOpenGLUpdateInfoSP;

class KRITAUI_EXPORT KisAnimationFrameCache : public QObject, public KisShared
{
    Q_OBJECT

public:

    static KisAnimationFrameCacheSP getFrameCache(KisOpenGLImageTexturesSP textures);
    static const QList<KisAnimationFrameCache*> caches();

    KisAnimationFrameCache(KisOpenGLImageTexturesSP textures);
    ~KisAnimationFrameCache() override;

    QImage getFrame(int time);
    bool uploadFrame(int time);

    bool shouldUploadNewFrame(int newTime, int oldTime) const;

    enum CacheStatus {
        Cached,
        Uncached,
    };

    CacheStatus frameStatus(int time) const;
    KisFrameSet cachedFramesWithin(KisTimeSpan range);
    int firstDirtyFrameWithin(KisTimeSpan range, const KisFrameSet *ignoredFrames = 0);

    KisImageWSP image();

    KisOpenGLUpdateInfoSP fetchFrameData(int time, KisImageSP image, const QRegion &requestedRegion) const;
    void addConvertedFrameData(KisOpenGLUpdateInfoSP info, int time);

    /**
     * Drops all the frames with worse level of detail values than the current
     * desired level of detail.
     */
    void dropLowQualityFrames(const KisTimeSpan &range, const QRect &regionOfInterest, const QRect &minimalRect);

Q_SIGNALS:
    void changed();

private:

    struct Private;
    QScopedPointer<Private> m_d;

private Q_SLOTS:
    void framesChanged(const KisFrameSet &range, const QRect &rect);
    void slotConfigChanged();
};

#endif
