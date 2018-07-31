/*
 *  Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_shape_selection_test.h"
#include <QTest>

#include <kis_debug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoPathShape.h>

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "flake/kis_shape_selection.h"
#include "kis_image.h"
#include "testutil.h"
#include "kistest.h"
#include <KisPart.h>
#include <KisDocument.h>

void KisShapeSelectionTest::testAddChild()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    QColor qc(Qt::white);
    qc.setAlpha(0);
    KoColor bgColor(qc, cs);
    doc->newImage("test", 300, 300, cs, bgColor, true, 1, "test", 100);
    KisImageSP image = doc->image();

    KisSelectionSP selection = new KisSelection();
    QVERIFY(selection->hasPixelSelection() == false);
    QVERIFY(selection->hasShapeSelection() == false);

    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    pixelSelection->select(QRect(0, 0, 100, 100));

    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 25, 25), MAX_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));

    QRect rect(50, 50, 100, 100);
    QTransform matrix;
    matrix.scale(1 / image->xRes(), 1 / image->yRes());
    rect = matrix.mapRect(rect);

    KoPathShape* shape = new KoPathShape();
    shape->setShapeId(KoPathShapeId);
    shape->moveTo(rect.topLeft());
    shape->lineTo(rect.topLeft() + QPointF(rect.width(), 0));
    shape->lineTo(rect.bottomRight());
    shape->lineTo(rect.topLeft() + QPointF(0, rect.height()));
    shape->close();
    shape->normalize();

    KisShapeSelection * shapeSelection = new KisShapeSelection(doc->shapeController(), image, selection);
    selection->setShapeSelection(shapeSelection);
    shapeSelection->addShape(shape);

    selection->updateProjection();
    image->waitForDone();

    QCOMPARE(selection->selectedExactRect(), QRect(50, 50, 100, 100));


}

KISTEST_MAIN(KisShapeSelectionTest)


