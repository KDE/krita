/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_filter_test.h"

#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_filter_processing_information.h"
#include "kis_filter.h"
#include "testutil.h"
#include "kis_threaded_applicator.h"

class TestFilter : public KisFilter 
{
public:

    TestFilter()
        : KisFilter(KoID("test", "test"), KoID("test", "test"), "TestFilter" )
        {
        }    
 
    using KisFilter::process;
 
    void process(KisFilterConstantProcessingInformation src,
                         KisFilterProcessingInformation dst,
                         const QSize& size,
                         const KisFilterConfiguration* config,
                         KoUpdater* progressUpdater) const
        {
            Q_UNUSED(src);
            Q_UNUSED(dst);
            Q_UNUSED(size);
            Q_UNUSED(config);
            Q_UNUSED(progressUpdater);
        }
 
};

void KisFilterTest::testCreation()
{
    TestFilter test;
}

void KisFilterTest::testSingleThreaded()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimg( QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage inverted( QString(FILES_DATA_DIR) + QDir::separator() + "inverted_hakonepa.png" );
    KisPaintDeviceSP dev = new KisPaintDevice(cs, "filter test");
    dev->convertFromQImage(qimg, "", 0, 0);
    
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT( f );

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT( kfc );

    KisFilterConstantProcessingInformation src( dev,  QPoint(0, 0), 0 );
    KisFilterProcessingInformation dst( dev, QPoint(0, 0), 0 );
    
    f->process(src, dst, qimg.size(), kfc);

    QPoint errpoint;
    if ( !TestUtil::compareQImages( errpoint, inverted, dev->convertToQImage(0, 0, 0, qimg.width(), qimg.height() ) ) ) {
        dev->convertToQImage(0, 0, 0, qimg.width(), qimg.height()).save("filtertest.png");
        QFAIL( QString( "Failed to create inverted image, first different pixel: %1,%2 " ).arg( errpoint.x() ).arg( errpoint.y() ).toAscii() );
    }
}

QTEST_KDEMAIN(KisFilterTest, GUI);
#include "kis_filter_test.moc"
