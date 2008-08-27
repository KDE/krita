/*
 *  Copyright (c) 2007 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_tiles_regression_tester.h"
#include <qtest_kde.h>


#include "kis_global.h"

#include "kis_paint_device.h"
#include "kis_layer.h"
#include "KoColorSpaceRegistry.h"

// ### HACK so that we can easily test the internals
#define private public
#define protected public

#undef private
#undef protected

static quint8 defPixel = 145;

void KisTilesRegressionTester::weirdCrash1Test()
{
    KisPaintDeviceSP p = new KisPaintDevice(0, KoColorSpaceRegistry::instance()->rgb8(), "regression test: weird crash");
}

QTEST_KDEMAIN(KisTilesRegressionTester, NoGUI)

#include "kis_tiles_regression_tester.moc"
