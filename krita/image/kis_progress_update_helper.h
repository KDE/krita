/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PROGRESS_UPDATE_HELPER_H
#define __KIS_PROGRESS_UPDATE_HELPER_H

#include <KoUpdater.h>

class KisProgressUpdateHelper {
public:
    KisProgressUpdateHelper(KoUpdaterPtr progressUpdater, int portion, int numSteps)
        : m_progressUpdater(progressUpdater),
          m_portion(portion),
          m_currentStep(0),
          m_numSteps(numSteps)
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

        if (m_progressUpdater) {
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
};

#endif /* __KIS_PROGRESS_UPDATE_HELPER_H */
