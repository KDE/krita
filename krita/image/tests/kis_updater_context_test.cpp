/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_updater_context_test.h"
#include <qtest_kde.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_updater_context.h"

void KisUpdaterContextTest::testJobInterference()
{
    KisTestableUpdaterContext context;

    QRect imageRect(0,0,100,100);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "merge test");

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);

    image->lock();
    image->addNode(paintLayer);

    QRect dirtyRect1(0,0,50,100);
    KisMergeWalkerSP walker1 = new KisMergeWalker(imageRect);
    walker1->collectRects(paintLayer, dirtyRect1);

    QRect dirtyRect2(30,0,100,100);
    KisMergeWalkerSP walker2 = new KisMergeWalker(imageRect);
    walker2->collectRects(paintLayer, dirtyRect2);

    context.lock();
    context.addJob(walker1);

    QVERIFY(!context.isJobAllowed(walker2));

    context.unlock();
}


QTEST_KDEMAIN(KisUpdaterContextTest, NoGUI)
#include "kis_updater_context_test.moc"

