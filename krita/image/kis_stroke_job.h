/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_STROKE_JOB_H
#define __KIS_STROKE_JOB_H

#include "kis_dab_processing_strategy.h"

class KisStrokeJob
{
public:
    KisStrokeJob(KisDabProcessingStrategy *strategy,
                 KisDabProcessingStrategy::DabProcessingData *data)
        : m_dabStrategy(strategy),
          m_dabData(data)
    {
    }

    ~KisStrokeJob() {
        delete m_dabData;
    }

    void run() {
        m_dabStrategy->processDab(m_dabData);
    }

    bool isSequential() const {
        return m_dabStrategy->isSequential();
    }

    bool isExclusive() const {
        return m_dabStrategy->isExclusive();
    }

private:
    // for testing use only, do not use in real code
    friend QString getJobName(KisStrokeJob *job);
    KisDabProcessingStrategy* testingGetDabStrategy() {
        return m_dabStrategy;
    }

private:
    // Shared between different jobs
    KisDabProcessingStrategy *m_dabStrategy;

    // Owned by the job
    KisDabProcessingStrategy::DabProcessingData *m_dabData;
};

#endif /* __KIS_STROKE_JOB_H */
