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

#ifndef KISASYNCANIMATIONRENDERDIALOGBASE_H
#define KISASYNCANIMATIONRENDERDIALOGBASE_H

#include <QObject>
#include "kis_types.h"
#include "kritaui_export.h"

class KisTimeRange;
class KisAsyncAnimationRendererBase;
class KisViewManager;

class KRITAUI_EXPORT KisAsyncAnimationRenderDialogBase : public QObject
{
    Q_OBJECT
public:
    enum Result {
        RenderComplete,
        RenderCancelled,
        RenderFailed
    };

public:
    KisAsyncAnimationRenderDialogBase(const QString &actionTitle, KisImageSP image, int busyWait = 200);
    virtual ~KisAsyncAnimationRenderDialogBase();

    virtual Result regenerateRange(KisViewManager *viewManager);

    void setBatchMode(bool value);
    bool batchMode() const;

private Q_SLOTS:
    void slotFrameCompleted(int frame);
    void slotFrameCancelled(int frame);

    void slotCancelRegeneration();

private:
    void tryInitiateFrameRegeneration();
    void updateProgressLabel();
    void cancelProcessingImpl(bool isUserCancelled);

protected:
    virtual QList<int> calcDirtyFrames() const = 0;
    virtual KisAsyncAnimationRendererBase* createRenderer(KisImageSP image) = 0;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISASYNCANIMATIONRENDERDIALOGBASE_H
