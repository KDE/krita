/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_IMAGE_ANIMATION_INTERFACE_H
#define __KIS_IMAGE_ANIMATION_INTERFACE_H

#include <QObject>
#include <QScopedPointer>

#include "kis_types.h"
#include "krita_export.h"


class KisUpdatesFacade;


class KRITAIMAGE_EXPORT KisImageAnimationInterface : public QObject
{
    Q_OBJECT

public:
    KisImageAnimationInterface(KisImage *image);
    ~KisImageAnimationInterface();

    int currentTime() const;
    void switchCurrentTime(int frameId) const;

    KisPaintDeviceSP frameProjection() const;

    void requestFrameRegeneration(int frameId, const QRegion &dirtyRegion, bool populateCache);
    void requestFrame(int frameId, bool populateCache);

private:
    // interface for:
    friend class KisRegenerateFrameStrokeStrategy;
    void setCurrentTimeExplicitly(int frameId);
    void notifyFrameReady(bool populateCache);
    KisUpdatesFacade* updatesFacade() const;

Q_SIGNALS:
    void sigFrameReady(bool populateCache);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_IMAGE_ANIMATION_INTERFACE_H */
