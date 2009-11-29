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

#include "kis_filter_job_test.h"

#include <qtest_kde.h>
#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "filter/kis_filter.h"
#include "testutil.h"
#include "kis_threaded_applicator.h"
#include "filter/kis_filter_job.h"
#include "testutil.h"
#include "kis_fill_painter.h"

void KisFilterJobTest::testCreation()
{
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr up = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    KisFilterJobFactory factory(f, kfc);
    ThreadWeaver::Job* job = factory.createJob(0, dev, QRect(0, 0, 2000, 2000), up);
    Q_ASSERT(job);
}


void KisFilterJobTest::testInWeaver()
{
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    TestUtil::TestProgressBar * bar = new TestUtil::TestProgressBar();
    KoProgressUpdater* pu = new KoProgressUpdater(bar);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + QDir::separator() + "inverted_hakonepa.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(qimage, "", 0, 0);

    KisFilterJobFactory factory(f, kfc);

    KisThreadedApplicator applicator(dev, QRect(0, 0, 2000, 2000), &factory, pu);
    applicator.execute();


    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtermasktest2.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
    delete pu;
    delete bar;
}

QTEST_KDEMAIN(KisFilterJobTest, GUI)
#include "kis_filter_job_test.moc"
