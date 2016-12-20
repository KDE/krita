/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_shape_commands_test.h"

#include <QTest>

#include "kis_global.h"

#include "kis_shape_layer.h"
#include <KoPathShape.h>
#include <KoColorBackground.h>
#include "testutil.h"

#include <KisPart.h>
#include <KisDocument.h>

#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>


void KisShapeCommandsTest::test()
{
    TestUtil::ExternalImageChecker chk("shape_commands_test", "grouping");

    QRect refRect(0,0,64,64);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    TestUtil::MaskParent p(refRect);

    const qreal resolution = 72.0 / 72.0;
    p.image->setResolution(resolution, resolution);

    doc->setCurrentImage(p.image);


    KisShapeLayerSP shapeLayer = new KisShapeLayer(doc->shapeController(), p.image, "shapeLayer1", 75);

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(5, 5));
        path->lineTo(QPointF(5, 55));
        path->lineTo(QPointF(55, 55));
        path->lineTo(QPointF(55,  5));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::red)));

        path->setName("shape1");
        path->setZIndex(1);
        shapeLayer->addShape(path);
    }

    {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);
        path->moveTo(QPointF(30, 30));
        path->lineTo(QPointF(30, 60));
        path->lineTo(QPointF(60, 60));
        path->lineTo(QPointF(60, 30));
        path->close();
        path->normalize();
        path->setBackground(toQShared(new KoColorBackground(Qt::green)));

        path->setName("shape2");
        path->setZIndex(2);
        shapeLayer->addShape(path);
    }

    p.image->addNode(shapeLayer);

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    QList<KoShape*> shapes = shapeLayer->shapes();

    KoShapeGroup *group = new KoShapeGroup();
    group->setName("group_shape");
    shapeLayer->addShape(group);

    QScopedPointer<KoShapeGroupCommand> cmd(
        new KoShapeGroupCommand(group, shapes, false, true, true));

    cmd->redo();

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    cmd->undo();

    shapeLayer->setDirty();
    qApp->processEvents();
    p.image->waitForDone();

    chk.checkImage(p.image, "00_initial_layer_update");

    QVERIFY(chk.testPassed());

}

QTEST_MAIN(KisShapeCommandsTest)
