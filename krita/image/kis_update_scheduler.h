/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_UPDATE_SCHEDULER_H
#define __KIS_UPDATE_SCHEDULER_H

#include "kis_abstract_update_scheduler.h"
#include "kis_updater_context.h"
#include "kis_abstract_update_queue.h"

class KRITAIMAGE_EXPORT KisUpdateScheduler : public KisAbstractUpdateScheduler
{
    Q_OBJECT

public:
    KisUpdateScheduler(KisImageWSP image);
    virtual ~KisUpdateScheduler();

    void lock();
    void unlock();
    void waitForDone();

    void updateProjection(KisNodeSP node, const QRect& rc);
    void fullRefresh(KisNodeSP root);

    void updateSettings();

protected slots:
    void doSomeUsefulWork();
    void spareThreadAppeared();

protected:
    KisImageWSP m_image;
    KisAbstractUpdateQueue* m_workQueue;
    KisUpdaterContext m_updaterContext;
};

#endif /* __KIS_UPDATE_SCHEDULER_H */

