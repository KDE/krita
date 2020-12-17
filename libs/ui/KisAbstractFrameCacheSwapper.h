/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISABSTRACTFRAMECACHESWAPPER_H
#define KISABSTRACTFRAMECACHESWAPPER_H

#include "kritaui_export.h"

class QRect;

template<class T>
class KisSharedPtr;

class KisOpenGLUpdateInfo;
typedef KisSharedPtr<KisOpenGLUpdateInfo> KisOpenGLUpdateInfoSP;


class KRITAUI_EXPORT KisAbstractFrameCacheSwapper
{
public:
    virtual ~KisAbstractFrameCacheSwapper();

    // WARNING: after transferring \p info to saveFrame() the object becomes invalid
    virtual void saveFrame(int frameId, KisOpenGLUpdateInfoSP info, const QRect &imageBounds) = 0;
    virtual KisOpenGLUpdateInfoSP loadFrame(int frameId) = 0;

    virtual void moveFrame(int srcFrameId, int dstFrameId) = 0;
    virtual void forgetFrame(int frameId) = 0;

    virtual bool hasFrame(int frameId) const = 0;

    virtual int frameLevelOfDetail(int frameId) const = 0;
    virtual QRect frameDirtyRect(int frameId) const = 0;
};

#endif // KISABSTRACTFRAMECACHESWAPPER_H
