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

/**
   A threadweaver job that knows about paint devices and rects. Note
   that it is the task of the job implementation to handle the margin!
 */
class KisJob : public ThreadWeaver::Job
{
public:

    KisJob( QObject * parent, KisPaintDeviceSP dev, const QRect & rc, int margin )
        : ThreadWeaver::Job( parent )
        , m_dev( dev )
        , m_rc( rc )
        , m_margin( margin )
        {
        }


    virtual ~KisJob() {}

    /// done() is called when the job is done
    virtual void jobDone() = 0;

protected:

    KisPaintDeviceSP m_dev;
    QRect m_rc;
    int m_margin;
};

/**
   Implement this interface to create the specific jobs you need.
 */
class KisJobFactory {

public:

    KisJobFactory() {}
    virtual ~KisJobFactory() {}


    virtual ThreadWeaver::Job * createJob(QObject * parent, KisPaintDeviceSP dev,  const QRect & rc, int margin) = 0;
};

/**
   The KisThreadedApplicator takes a paint device, a job factory and a
   paint device and creates threadweaver jobs for as many subrects as
   are needed to cover the whole paint device.
*/
class KisThreadedApplicator : public QObject {

    Q_OBJECT

public:

    /**
       @param dev The paintdevice that is the subject of the jobs
       @param rc The part of the paintdevice that needs to be acted on
       @param jobFactory The factory class that creates the
              specialized jobs
       @param margin. If present, the rects parcelled out to the jobs
                      will have the specified margin. When the results
                      are put together again, the margin is cut off.
                      Use this for convolutions, for instance.
    */
    KisThreadedApplicator( KisPaintDeviceSP dev, const QRect & rc, KisJobFactory * jobFactory, int margin = 0 );
    ~KisThreadedApplicator();

    /**
      Start all threads. Returns when all subthreads are done.

      XXX: make it possible to cancel the jobs!
      XXX: integrate with the progress updater thingy

     */
    void execute();

private slots:

    void jobDone( ThreadWeaver::Job* );

private:

    class Private;
    Private * m_d;

};
#endif
