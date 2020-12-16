/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PROGRESS_UPDATE_HELPER_H
#define __KIS_PROGRESS_UPDATE_HELPER_H

#include <KoUpdater.h>
#include <kis_types.h>

class KisProgressUpdateHelper {
public:
    KisProgressUpdateHelper(KoUpdaterPtr progressUpdater, int portion, int numSteps)
        : m_progressUpdater(progressUpdater),
          m_baseProgress(0),
          m_portion(portion),
          m_currentStep(0),
          m_numSteps(numSteps),
          m_lastReportedLocalProgress(-1)
     {
         if (m_progressUpdater) {
             m_baseProgress = m_progressUpdater->progress();
         }
     }

    ~KisProgressUpdateHelper() {
        if (m_progressUpdater) {
            m_progressUpdater->setProgress(m_baseProgress + m_portion);
        }
    }

    void step() {
        int localProgress = m_numSteps ?
            m_portion * (++m_currentStep) / m_numSteps : m_portion;

        if (m_progressUpdater && m_lastReportedLocalProgress != localProgress) {
            m_lastReportedLocalProgress = localProgress;
            m_progressUpdater->setProgress(m_baseProgress + localProgress);
        }
        // TODO: handle interrupted processing (connect to other layers, i.e. undo)
    }

private:
    KoUpdaterPtr m_progressUpdater;
    int m_baseProgress;
    int m_portion;
    int m_currentStep;
    int m_numSteps;
    int m_lastReportedLocalProgress;
};

#endif /* __KIS_PROGRESS_UPDATE_HELPER_H */
