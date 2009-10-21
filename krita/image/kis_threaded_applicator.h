/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org
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
#ifndef KIS_THREADED_APPLICATOR
#define KIS_THREADED_APPLICATOR

#include <QObject>
#include <QRect>

#include <threadweaver/Job.h>

#include "kis_types.h"
#include <krita_export.h>

#include <KoProgressUpdater.h>

typedef QPointer<KoUpdater> KoUpdaterPtr;

/**
 * A threadweaver job that knows about paint devices and rects.
 *
 * Any subclass of KisJob is responsible for setting the interrupted
 * status when applicable.
 */
class KRITAIMAGE_EXPORT KisJob : public ThreadWeaver::Job
{
public:

    KisJob(QObject * parent, KisPaintDeviceSP dev, const QRect & rc)
            : ThreadWeaver::Job(parent)
            , m_dev(dev)
            , m_rc(rc) {
    }


    virtual ~KisJob() {}

    /**
     * Reimplement this method if you need something done whenever the
     * job is done.
     */
    virtual void jobDone() {}

    /**
     * return the area affected by the job
     */
    QRect area() {
        return m_rc;
    }

    /**
     * @return true if the job got cancelled.
     */
    bool interrupted() {
        return m_interrupted;
    }

protected:

    KisPaintDeviceSP m_dev;
    QRect m_rc;
    bool m_interrupted;
};

/**
   Implement this interface to create the specific jobs you need.
 */
class KRITAIMAGE_EXPORT KisJobFactory
{

public:

    KisJobFactory() {}
    virtual ~KisJobFactory() {}


    virtual ThreadWeaver::Job * createJob(QObject * parent, KisPaintDeviceSP dev,  const QRect & rc, KoUpdaterPtr updater) = 0;
};

/**
 * The KisThreadedApplicator takes a paint device, a job factory and a
 * paint device and creates threadweaver jobs for as many subrects as
 *  are needed to cover the whole paint device.
 *
 * XXX: make it use kissystemlocker & threadweaver jobcollections
 */
class KRITAIMAGE_EXPORT KisThreadedApplicator : public QObject
{

    Q_OBJECT

public:


    enum ApplicatorMode {
        TILED,
        UNTILED
    };


    /**
     * @param dev The paintdevice that is the subject of the jobs
     *  @param rc The part of the paintdevice that needs to be acted on
     *  @param jobFactory The factory class that creates the
     *         specialized jobs
     *  @param updater The master KoProgressUpdater that will track updates for
     *         all threads.
     */
    KisThreadedApplicator(KisPaintDeviceSP dev, const QRect & rc,
                          KisJobFactory * jobFactory, KoProgressUpdater * updater,
                          ApplicatorMode mode = TILED);
    ~KisThreadedApplicator();

    /**
     * Execute the given job and return when the whole rect has
     * been done.
     */
    void execute();

    /**
     * Queue all the subtasks;
     */
    void start();

signals:

    /// emitted whenever an area is done
    void areaDone(const QRect& rc);

    /**
     * emitted when all the subtasks are done.
     * If interrupted is true, a subtask got cancelled. Note that if the updater
     * is interrupted, all subtasks will be interrupted. This signal is emitted
     * only after all subtasks have definitely stopped.
     */
    void finished(bool interrupted);

private slots:

    void jobDone(ThreadWeaver::Job*);
    void applicationQuit();

private:

    class Private;
    Private * const m_d;

};
#endif
