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
 * A threadweaver job that knows about paint devices and rects. Note
 * that it is the task of the job implementation to handle the margin!
 */
class KRITAIMAGE_EXPORT KisJob : public ThreadWeaver::Job
{
public:

    KisJob(QObject * parent, KisPaintDeviceSP dev, const QRect & rc, int margin)
            : ThreadWeaver::Job(parent)
            , m_dev(dev)
            , m_rc(rc)
            , m_margin(margin) {
    }


    virtual ~KisJob() {}

    /**
     * Reimplement this method if you need something done whenever the
     * job is done.
     */
    virtual void jobDone() {}

    QRect area() {
        return m_rc;
    }
protected:

    KisPaintDeviceSP m_dev;
    QRect m_rc; // the area we execute on without the margin
    int m_margin; // we will execute on m_rc enlarged by the margin
};

/**
   Implement this interface to create the specific jobs you need.
 */
class KRITAIMAGE_EXPORT KisJobFactory
{

public:

    KisJobFactory() {}
    virtual ~KisJobFactory() {}


    virtual ThreadWeaver::Job * createJob(QObject * parent, KisPaintDeviceSP dev,  const QRect & rc, int margin, KoUpdaterPtr updater) = 0;
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
     *  @param margin. If present, the rects parcelled out to the jobs
     *                 will have the specified margin. When the results
     *                 are put together again, the margin is cut off.
     *                 Use this for convolutions, for instance.
     *
     */
    KisThreadedApplicator(KisPaintDeviceSP dev, const QRect & rc,
                          KisJobFactory * jobFactory, KoProgressUpdater * updater,
                          int margin = 0, ApplicatorMode mode = TILED);
    ~KisThreadedApplicator();

    /**
     * Execute the given job and return when the whole rect has
     * been done.
     */
    void execute();

signals:

    void areaDone(const QRect& rc);

private slots:

    void jobDone(ThreadWeaver::Job*);

private:

    class Private;
    Private * const m_d;

};
#endif
