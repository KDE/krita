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

#include "kis_paint_device_jobs_test.h"

#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device_jobs.h"

void KisPaintDeviceJobsTest::testCreation()
{
    const KoColorSpace * srcCs = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace * dstCs = KoColorSpaceRegistry::instance()->lab16();

    quint8* src = new quint8[ 64 * 4 ];
    memset(src, 0, 64 * 4);
    quint8* dst = new quint8[ 64 * 8 ];
    memset(dst, 0, 64 * 8);

    ConversionJob test(src, dst, srcCs, dstCs, 64,  KoColorConversionTransformation::IntentPerceptual, 0);

    delete[] src;
    delete[] dst;
}


QTEST_KDEMAIN(KisPaintDeviceJobsTest, GUI)
#include "kis_paint_device_jobs_test.moc"
