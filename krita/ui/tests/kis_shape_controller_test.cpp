/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

#include "kis_shape_controller_test.h"
#include <QTest>
#include <QCoreApplication>
#include <QSignalSpy>

#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kis_debug.h>

#include "KoColorSpace.h"
#include "KoCompositeOp.h"
#include "KoColorSpaceRegistry.h"

#include "kis_doc2.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_node_model.h"
#include "kis_name_server.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_group_layer.h"
#include "flake/kis_shape_controller.h"
#include "kis_types.h"
#include "kis_transparency_mask.h"

#include "node_shapes_utils.h"


void KisShapeControllerTest::init()
{
    m_doc = new KisDoc2;
    m_nameServer = new KisNameServer();
    m_shapeController = new KisShapeController(m_doc, m_nameServer);

    m_image = new KisImage(0, 512, 512, 0, "test");
    m_layer1 = new KisPaintLayer(m_image, "layer1", OPACITY_OPAQUE_U8);
    m_layer2 = new KisGroupLayer(m_image, "layer2", OPACITY_OPAQUE_U8);
    m_layer3 = new KisCloneLayer(m_layer1, m_image, "layer3", OPACITY_OPAQUE_U8);
    m_layer4 = new KisGroupLayer(m_image, "layer4", OPACITY_OPAQUE_U8);
    m_mask1 = new KisTransparencyMask();
}

void KisShapeControllerTest::cleanup()
{
    m_layer1 = 0;
    m_layer2 = 0;
    m_layer3 = 0;
    m_layer4 = 0;
    m_mask1 = 0;
    m_image = 0;

    delete m_shapeController;
    delete m_nameServer;
    delete m_doc;
}

void KisShapeControllerTest::constructImage()
{
    m_image->addNode(m_layer1);
    m_image->addNode(m_layer2);
    m_image->addNode(m_layer3);
    m_image->addNode(m_layer4);
    m_image->addNode(m_mask1, m_layer3);
}

void KisShapeControllerTest::testSetImage()
{
    constructImage();

    m_shapeController->setImage(m_image);

    QString actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    QString expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_shapeController->setImage(0);
    QCOMPARE(m_shapeController->layerMapSize(), 0);
}

void KisShapeControllerTest::testAddNode()
{
    QString actualGraph;
    QString expectedGraph;

    m_shapeController->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 1);

    constructImage();

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_shapeController->setImage(0);
    QCOMPARE(m_shapeController->layerMapSize(), 0);

    m_shapeController->setImage(0);
}

void KisShapeControllerTest::testRemoveNode()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_shapeController->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_image->removeNode(m_layer2);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 5);

    m_image->removeNode(m_layer3);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 3);

    m_shapeController->setImage(0);
}

void KisShapeControllerTest::testMoveNodeSameParent()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_shapeController->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_image->moveNode(m_layer2, m_image->root(), m_layer3);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer3 effect layer2 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_shapeController->setImage(0);
}

void KisShapeControllerTest::testMoveNodeDifferentParent()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_shapeController->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_image->moveNode(m_layer2, m_image->root(), m_layer4);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer3 effect layer4 layer2";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_image->moveNode(m_layer3, m_layer4, m_layer4->lastChild());

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer4 layer3 effect layer2";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_shapeController->setImage(0);
}

void KisShapeControllerTest::testSubstituteRootNode()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_shapeController->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 6);

    m_image->flatten();

    actualGraph = collectGraphPatternFull(m_shapeController->dummyForNode(m_image->root()));
    expectedGraph = "root Layer 1";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_shapeController->layerMapSize(), 2);

    m_shapeController->setImage(0);
}


QTEST_KDEMAIN(KisShapeControllerTest, GUI)
#include "kis_shape_controller_test.moc"
