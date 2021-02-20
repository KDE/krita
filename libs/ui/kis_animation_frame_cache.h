/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class KisRegion;

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
    static const KisAnimationFrameCacheSP cacheForImage(KisImageWSP image);

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

    KisImageWSP image();

    KisOpenGLUpdateInfoSP fetchFrameData(int time, KisImageSP image, const KisRegion &requestedRegion) const;
    void addConvertedFrameData(KisOpenGLUpdateInfoSP info, int time);

    /**
     * Drops all the frames with worse level of detail values than the current
     * desired level of detail.
     */
    void dropLowQualityFrames(const KisTimeSpan &range, const QRect &regionOfInterest, const QRect &minimalRect);

    bool framesHaveValidRoi(const KisTimeSpan &range, const QRect &regionOfInterest);

Q_SIGNALS:
    void changed();

private:

    struct Private;
    QScopedPointer<Private> m_d;

private Q_SLOTS:
    void framesChanged(const KisTimeSpan &range, const QRect &rect);
    void slotConfigChanged();
};

#endif
