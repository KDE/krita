/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisHoldUIUpdatesCommand.h"

#include <algorithm>
#include "kis_image_interfaces.h"
#include "krita_utils.h"
#include "kis_paintop_utils.h"
#include "kis_image_signal_router.h"
#include "KisRunnableStrokeJobData.h"
#include "KisRunnableStrokeJobUtils.h"
#include "KisRunnableStrokeJobsInterface.h"


KisHoldUIUpdatesCommand::KisHoldUIUpdatesCommand(KisUpdatesFacade *updatesFacade, State state)
    : KisCommandUtils::FlipFlopCommand(state),
      m_updatesFacade(updatesFacade),
      m_batchUpdateStarted(new bool(false))
{
}

void KisHoldUIUpdatesCommand::partA()
{
    if (*m_batchUpdateStarted) {
        m_updatesFacade->notifyBatchUpdateEnded();
        *m_batchUpdateStarted = false;
    }

    m_updatesFacade->disableUIUpdates();
}

void KisHoldUIUpdatesCommand::partB()
{
    QVector<QRect> totalDirtyRects = m_updatesFacade->enableUIUpdates();

    const QRect totalRect =
        m_updatesFacade->bounds() &
        std::accumulate(totalDirtyRects.begin(), totalDirtyRects.end(), QRect(), std::bit_or<QRect>());

    totalDirtyRects =
        KisPaintOpUtils::splitAndFilterDabRect(totalRect,
                                               totalDirtyRects,
                                               KritaUtils::optimalPatchSize().width());

    *m_batchUpdateStarted = true;
    m_updatesFacade->notifyBatchUpdateStarted();

    KisUpdatesFacade *updatesFacade = m_updatesFacade;
    QSharedPointer<bool> batchUpdateStarted = m_batchUpdateStarted;

    QVector<KisRunnableStrokeJobDataBase*> jobsData;
    Q_FOREACH (const QRect &rc, totalDirtyRects) {
        KritaUtils::addJobConcurrent(jobsData, [updatesFacade, rc] () {
            updatesFacade->notifyUIUpdateCompleted(rc);
        });
    }

    KritaUtils::addJobBarrier(jobsData, [updatesFacade, batchUpdateStarted] () {
        updatesFacade->notifyBatchUpdateEnded();
        *batchUpdateStarted = false;
    });

    runnableJobsInterface()->addRunnableJobs(jobsData);
}
