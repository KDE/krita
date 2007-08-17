/*
 *  kis_tool_freehand_p.h - part of Krita
 *
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_TOOL_FREEHAND_P_H
#define KIS_TOOL_FREEHAND_P_H

#include <QThread>
#include <QPointF>
#include <QMutex>
#include <QQueue>
#include <QMutexLocker>

#include <kdebug.h>
#include <kis_paint_information.h>

class KisToolFreehand;
class KisPainter;


/**
 * Private classes for use by the freehand tool
 */

/**
 * XXX: doc
 */
class FreehandPaintJob {

public:

    FreehandPaintJob(KisToolFreehand* freeHand,
                     KisPainter* painter,
                     const KisPaintInformation & pi1,
                     const KisPaintInformation & pi2,
                     const FreehandPaintJob* previousPaintJob);

    virtual ~FreehandPaintJob();

    double dragDist() const { return m_dragDist; }
    virtual void run() = 0;

protected:

    KisToolFreehand* m_toolFreeHand;
    KisPainter* m_painter;
    double m_dragDist;
    KisPaintInformation m_pi1;
    KisPaintInformation m_pi2;
    const FreehandPaintJob* m_previousPaintJob;

};

/**
 * XXX: doc
 */
class FreehandPaintLineJob : public FreehandPaintJob {

public:

    FreehandPaintLineJob(KisToolFreehand* freeHand,
                         KisPainter* painter,
                         const KisPaintInformation & pi1,
                         const KisPaintInformation & pi2,
                         const FreehandPaintJob* previousPaintJob);
    virtual ~FreehandPaintLineJob();
    virtual void run();
};


/**
 * XXX: doc
 */
class FreehandPaintBezierJob : public FreehandPaintJob {

public:

    FreehandPaintBezierJob(KisToolFreehand* freeHand,
                           KisPainter* painter,
                           const KisPaintInformation & pi1,
                           const QPointF& control1,
                           const QPointF& control2,
                           const KisPaintInformation & pi2,
                           const FreehandPaintJob* previousPaintJob);

    virtual ~FreehandPaintBezierJob();

    virtual void run();

private:

    QPointF m_control1;
    QPointF m_control2;

};

class FreehandPaintJobExecutor : public QThread {

public:

    FreehandPaintJobExecutor()
        : m_finish(false)
        {
        }

    virtual void run()
        {
            QMutexLocker lockRunning(&m_mutex_running);
            kDebug(41007) <<"run";
            while(not m_finish or not empty() )
            {
                FreehandPaintJob* nextJob = 0;
                {
                    QMutexLocker lock(&m_mutex_queue);
                    if(m_queue.size() > 0)
                    {
                        nextJob = m_queue.dequeue();
                    }
                }
                kDebug(41007) <<"nextJob =" << nextJob;
                if(nextJob)
                {
                    nextJob->run();
                } else {
                    msleep(1);
                }
            }
            kDebug(41007) <<"finish running";
        }
        void postJob(FreehandPaintJob* job)
        {
            QMutexLocker lock(&m_mutex_queue);
            kDebug(41007) <<"push job =" << job;
            m_queue.enqueue(job);
        }
        void finish() {
            m_finish = true;
            QMutexLocker lockRunning(&m_mutex_running);
        }
        bool empty() {
            QMutexLocker lock(&m_mutex_queue);
            return m_queue.size() == 0;
        }
        int queueLength() {
            QMutexLocker lock(&m_mutex_queue);
            return m_queue.size();
        }
        void start() {
            m_finish = false;
            QThread::start();
        }
    private:
        QQueue<FreehandPaintJob* > m_queue;
        QMutex m_mutex_queue;
        QMutex m_mutex_running;
        bool m_finish;
};



#endif
