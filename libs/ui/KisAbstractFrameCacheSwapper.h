/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
