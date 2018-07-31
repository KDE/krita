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

#include "kis_cs_conversion_test.h"
#include <QTest>

#include <QTime>

#include <KoColor.h>
#include <KoColorSpace.h>

#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_datamanager.h"
#include "kis_global.h"
#include "testutil.h"
#include "kis_transaction.h"
#include "kis_image.h"
#include "testing_timed_default_bounds.h"

void logFailure(const QString & reason, const KoColorSpace * srcCs, const KoColorSpace * dstCs)
{
    QString profile1("no profile");
    QString profile2("no profile");
    if (srcCs->profile())
        profile1 = srcCs->profile()->name();
    if (dstCs->profile())
        profile2 = dstCs->profile()->name();

    QWARN(QString("Failed %1 %2 -> %3 %4 %5")
          .arg(srcCs->name())
          .arg(profile1)
          .arg(dstCs->name())
          .arg(profile2)
          .arg(reason)
          .toLatin1());
}

void KisCsConversionTest::testColorSpaceConversion()
{
    QTime t;
    t.start();

    QList<const KoColorSpace*> colorSpaces = KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);
    int failedColorSpaces = 0;

    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");

    Q_FOREACH (const KoColorSpace * srcCs, colorSpaces) {
        Q_FOREACH (const KoColorSpace * dstCs,  colorSpaces) {

            KisPaintDeviceSP dev  = new KisPaintDevice(srcCs);
            dev->convertFromQImage(image, 0);
            dev->moveTo(10, 10);   // Unalign with tile boundaries
            dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(dev->exactBounds()));
            delete dev->convertTo(dstCs);

            if (dev->exactBounds() != QRect(10, 10, image.width(), image.height())) {
                logFailure("bounds", srcCs, dstCs);
                failedColorSpaces++;
            }
            if (dev->pixelSize() != dstCs->pixelSize()) {
                logFailure("pixelsize", srcCs, dstCs);
                failedColorSpaces++;
            }
            if (*dev->colorSpace() != *dstCs) {
                logFailure("dest cs", srcCs, dstCs);
                failedColorSpaces++;
            }
        }
    }
    qDebug() << colorSpaces.size() * colorSpaces.size()
    << "conversions"
    << " done in "
    << t.elapsed()
    << "ms";

    if (failedColorSpaces > 0) {
        QFAIL(QString("Failed conversions %1, see log for details.").arg(failedColorSpaces).toLatin1());
    }
}

KISTEST_MAIN(KisCsConversionTest)


