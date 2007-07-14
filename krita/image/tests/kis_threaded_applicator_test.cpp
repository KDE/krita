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

            quint8 * oldBytes = m_dev->colorSpace()->allocPixelBuffer( 1 );
            memset( oldBytes, 128, m_dev->colorSpace()->pixelSize() );

            quint8 * newBytes = m_dev->colorSpace()->allocPixelBuffer( 1 );
            memset( newBytes, 255, m_dev->colorSpace()->pixelSize() );

            QRect rc = m_rc.adjusted( -m_margin, -m_margin, m_margin, m_margin );

            {
                KisRectIteratorPixel it = m_dev->createRectIterator(rc.x(), rc.y(), rc.width(), rc.height() );
                while ( !it.isDone() ) {
                    QVERIFY( memcmp(it.oldRawData(), oldBytes, m_dev->colorSpace()->pixelSize() ) == 0 );
                    ++it;
                }
            }

            {
                KisRectIteratorPixel it = m_dev->createRectIterator(m_rc.x(), m_rc.y(), m_rc.width(), m_rc.height() );
                while ( !it.isDone() ) {
                    memcpy(it.rawData(), newBytes, m_dev->colorSpace()->pixelSize() );
                    ++it;
                }
            }

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

    quint8 *bytes = test->colorSpace()->allocPixelBuffer( 1 );
    memset( bytes, 128, test->colorSpace()->pixelSize() );
    test->fill( 0, 0, 1000, 1000, bytes );

    KisThreadedApplicator applicator( test, QRect( 0, 0, 1000, 1000 ), &factory);
    applicator.execute();

    KisRectConstIteratorPixel it = test->createRectConstIterator(0, 0, 1000, 1000 );
    while ( !it.isDone() ) {
        QCOMPARE( ( int )it.rawData()[0], ( int )255 );
        QCOMPARE( ( int )it.rawData()[1], ( int )255 );
        QCOMPARE( ( int )it.rawData()[2], ( int )255 );
        QCOMPARE( ( int )it.rawData()[3], ( int )255 );
        ++it;
    }
}

QTEST_KDEMAIN(KisThreadedApplicatorTest, NoGUI)
#include "kis_threaded_applicator_test.moc"


