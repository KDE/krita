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

#ifndef KISASYNCANIMATIONRENDERERBASE_H
#define KISASYNCANIMATIONRENDERERBASE_H

#include <QObject>
#include "kis_types.h"

class KisAsyncAnimationRendererBase : public QObject
{
    Q_OBJECT
public:
    explicit KisAsyncAnimationRendererBase(QObject *parent = 0);
    virtual ~KisAsyncAnimationRendererBase();

    virtual void startFrameRegeneration(KisImageSP image, int frame);

    bool isActive() const;
    KisImageSP requestedImage() const;

public Q_SLOTS:
    void cancelCurrentFrameRendering();

Q_SIGNALS:
    void sigFrameCompleted(int frame);
    void sigFrameCancelled(int frame);

private Q_SLOTS:
    void slotFrameRegenerationCancelled();
    void slotFrameRegenerationFinished(int frame);

protected Q_SLOTS:
    void notifyFrameCompleted(int frame);
    void notifyFrameCancelled(int frame);

protected:
    virtual void frameCompletedCallback(int frame) = 0;
    virtual void frameCancelledCallback(int frame) = 0;

protected:
    int requestedFrame() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISASYNCANIMATIONRENDERERBASE_H
