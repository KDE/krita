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
#include <qtest_kde.h>

#include <kis_debug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoPathShape.h>

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "flake/kis_shape_selection.h"
#include "kis_image.h"
#include "kis_undo_adapter.h"

void KisShapeSelectionTest::testAddChild()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(new KisUndoAdapter(0), 300, 300, cs, "test");

    KisSelectionSP selection = new KisSelection();
    QVERIFY(selection->hasPixelSelection() == false);
    QVERIFY(selection->hasShapeSelection() == false);
    KisPixelSelectionSP pixelSelection = selection->getOrCreatePixelSelection();

    pixelSelection->select(QRect(0, 0, 100, 100));
    // Selection is using the pixel selection as datamanager so no projection update
    // needed
    QCOMPARE(pixelSelection->selected(25, 25), MAX_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 100, 100));

    QRect rect(50, 50, 100, 100);
    QMatrix matrix;
    matrix.scale(1 / image->xRes(), 1 / image->yRes());
    rect = matrix.map(rect);

    KoPathShape* shape = new KoPathShape();
    shape->setShapeId(KoPathShapeId);
    shape->moveTo(rect.topLeft());
    shape->lineTo(rect.topLeft() + QPointF(rect.width(), 0));
    shape->lineTo(rect.bottomRight());
    shape->lineTo(rect.topLeft() + QPointF(0, rect.height()));
    shape->close();
    shape->normalize();

    KisShapeSelection * shapeSelection = new KisShapeSelection(image, selection);
    selection->setShapeSelection(shapeSelection);
    shapeSelection->addChild(shape);

    QCOMPARE(pixelSelection->selected(25, 25), MAX_SELECTED);
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 150, 150));

    selection->updateProjection();
}

QTEST_KDEMAIN(KisShapeSelectionTest, NoGUI)
#include "kis_shape_selection_test.moc"


