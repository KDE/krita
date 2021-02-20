/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
