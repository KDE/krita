/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <qtest_kde.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_threaded_applicator_test.h"
#include "kis_threaded_applicator.h"

class TestJob : public KisJob {
public:
    TestJob(   QObject * parent, KisPaintDeviceSP dev, const QRect & rc, int margin  )
        : KisJob( parent, dev, rc, margin )
        {
        }

    virtual void run()
        {
        }

    virtual void jobDone()
        {
        }
};

class TestJobFactory : public KisJobFactory {
public:
    ThreadWeaver::Job * createJob(QObject * parent, KisPaintDeviceSP dev,  const QRect & rc, int margin)
        {
            return new TestJob( parent, dev, rc, margin );
        }
};

void KisThreadedApplicatorTest::testApplication()
{
    KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    TestJobFactory factory;

    KisPaintDeviceSP test = new KisPaintDevice( colorSpace, "test" );
    KisThreadedApplicator applicator( test, QRect( 0, 0, 1000, 1000 ), &factory);
//    connect( &applicator, SIGNAL( jobDone( ThreadWeaver::Job ) ),
//    this, SLOT( jobDone() ) );
    applicator.execute();
}

QTEST_KDEMAIN(KisThreadedApplicatorTest, NoGUI)
#include "kis_threaded_applicator_test.moc"


