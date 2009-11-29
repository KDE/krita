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

#include "kis_transform_visitor_test.h"

#include <qtest_kde.h>
#include "kis_transform_visitor.h"

#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_types.h"
#include "kis_image.h"
#include "kis_filter_strategy.h"
#include "testutil.h"
#include "kis_image.h"


void KisTransformVisitorTest::testCreation()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 10, 10, cs, "bla");
    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransformVisitor test(image, 1.5, 1.5, 0, 0, 1.5, 0, 0, updater, filter);
    delete filter;
}


QTEST_KDEMAIN(KisTransformVisitorTest, GUI)
#include "kis_transform_visitor_test.moc"
