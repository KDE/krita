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

#ifndef __KIS_UPDATE_JOB_ITEM_H
#define __KIS_UPDATE_JOB_ITEM_H

#include <QRunnable>
#include <QReadWriteLock>

#include "kis_dab_processing_strategy.h"
#include "kis_base_rects_walker.h"
#include "kis_async_merger.h"


class KisUpdateJobItem :  public QObject, public QRunnable
{
    Q_OBJECT
public:
    enum Type {
        EMPTY,
        MERGE,
        STROKE
    };

public:
    KisUpdateJobItem(QReadWriteLock *exclusiveJobLock)
        : m_exclusiveJobLock(exclusiveJobLock),
          m_type(EMPTY)
    {
        setAutoDelete(false);
    }

    void run() {
        if(m_isExclusive) {
            m_exclusiveJobLock->lockForWrite();
        } else {
            m_exclusiveJobLock->lockForRead();
        }

        if(m_type == MERGE) {
            runMergeJob();
        } else {
            runStrokeJob();
        }

        setDone();

        emit sigDoSomeUsefulWork();
        emit sigJobFinished();

        m_exclusiveJobLock->unlock();
    }

    inline void runMergeJob() {
        Q_ASSERT(m_type == MERGE);
//        qDebug() << "Executing merge job" << m_walker->changeRect()
//                 << "on thread" << QThread::currentThreadId();
        m_merger.startMerge(*m_walker);

        QRect changeRect = m_walker->changeRect();
        emit sigContinueUpdate(changeRect);
    }

    inline void runStrokeJob() {
        Q_ASSERT(m_type == STROKE);
//        qDebug() << "Executing stroke job" << m_dabStrategy << m_dabData
//                 << "on thread" << QThread::currentThreadId();
        m_dabStrategy->processDab(m_dabData);
    }

    inline void setWalker(KisBaseRectsWalkerSP walker) {
        m_type = MERGE;
        m_accessRect = walker->accessRect();
        m_changeRect = walker->changeRect();
        m_walker = walker;

        m_isExclusive = false;
        m_dabStrategy = 0;
        m_dabData = 0;
    }

    inline void setStrokeJob(KisDabProcessingStrategy *strategy,
                             KisDabProcessingStrategy::DabProcessingData *data) {
        m_type = STROKE;
        m_dabStrategy = strategy;
        m_dabData = data;

        m_isExclusive = strategy->isExclusive();
        m_walker = 0;
        m_accessRect = m_changeRect = QRect();
    }

    inline void setDone() {
        m_type = EMPTY;
    }

    inline bool isRunning() const {
        return m_type != EMPTY;
    }

    inline const QRect& accessRect() const {
        return m_accessRect;
    }

    inline const QRect& changeRect() const {
        return m_changeRect;
    }

signals:
    void sigContinueUpdate(const QRect& rc);
    void sigDoSomeUsefulWork();
    void sigJobFinished();

private:
    /**
     * Open walker for the testing suite.
     * Please, do not use it in production code.
     */
    friend class KisSimpleUpdateQueueTest;
    inline KisBaseRectsWalkerSP walker() const {
        return m_walker;
    }

private:
    /**
     * \see KisUpdaterContext::m_exclusiveJobLock
     */
    QReadWriteLock *m_exclusiveJobLock;

    bool m_isExclusive;

    volatile Type m_type;

    /**
     * Stroke jobs part
     */

    KisDabProcessingStrategy *m_dabStrategy;
    KisDabProcessingStrategy::DabProcessingData *m_dabData;


    /**
     * Merge jobs part
     */

    KisBaseRectsWalkerSP m_walker;
    KisAsyncMerger m_merger;

    /**
     * These rects cache actual values from the walker
     * to eliminate concurrent access to a walker structure
     */
    QRect m_accessRect;
    QRect m_changeRect;
};


#endif /* __KIS_UPDATE_JOB_ITEM_H */
