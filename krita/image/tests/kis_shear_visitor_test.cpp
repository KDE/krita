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

#include "kis_shear_visitor_test.h"

#include "kis_shear_visitor.h"

#include "testutil.h"
#include <KoProgressUpdater.h>
#include "kis_image.h"
#include "kis_paint_device.h"

void KisShearVisitorTest::testCreation()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdater updater = pu.startSubtask();
    KisShearVisitor test(0.5, 0.5, &updater);
}


QTEST_KDEMAIN(KisShearVisitorTest, GUI)
#include "kis_shear_visitor_test.moc"
