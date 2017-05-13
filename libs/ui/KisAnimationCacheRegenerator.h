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

#ifndef KISANIMATIONCACHEREGENERATOR_H
#define KISANIMATIONCACHEREGENERATOR_H

#include <QObject>
#include <QScopedPointer>
#include "kritaui_export.h"
#include "kis_types.h"

class KisTimeRange;


class KRITAUI_EXPORT KisAnimationCacheRegenerator : public QObject
{
    Q_OBJECT
public:
    explicit KisAnimationCacheRegenerator(QObject *parent = 0);
    ~KisAnimationCacheRegenerator() override;

    static int calcFirstDirtyFrame(KisAnimationFrameCacheSP cache,
                                   const KisTimeRange &playbackRange,
                                   const KisTimeRange &skipRange);
    static int calcNumberOfDirtyFrame(KisAnimationFrameCacheSP cache,
                                      const KisTimeRange &playbackRange);


public Q_SLOTS:
    void startFrameRegeneration(int frame, KisAnimationFrameCacheSP cache);
    void cancelCurrentFrameRegeneration();

Q_SIGNALS:
    void sigFrameCancelled();
    void sigFrameFinished();

    void sigInternalStartFrameConversion();

private Q_SLOTS:
    void slotFrameRegenerationCancelled();
    void slotFrameRegenerationFinished(int frame);
    void slotFrameStartConversion();
    void slotFrameConverted();


private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISANIMATIONCACHEREGENERATOR_H
