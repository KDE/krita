
/*
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_model_test.h"

#include <QTest>
#include <kis_debug.h>

#include "KisDocument.h"
#include "KisPart.h"
#include "kis_node_model.h"
#include "kis_name_server.h"
#include "flake/kis_shape_controller.h"

#include <sdk/tests/testutil.h>
#include <sdk/tests/testui.h>
#include <sdk/tests/testui.h>

#include "modeltest.h"

void KisNodeModelTest::init()
{
    m_doc = KisPart::instance()->createDocument();

    m_nameServer = new KisNameServer();
    m_shapeController = new KisShapeController(m_nameServer, m_doc->undoStack());
    m_nodeModel = new KisNodeModel(0);

    initBase();

    m_doc->setCurrentImage(m_image);
}

void KisNodeModelTest::cleanup()
{
    cleanupBase();

    delete m_nodeModel;
    delete m_shapeController;
    delete m_nameServer;
    delete m_doc;
}

void KisNodeModelTest::testSetImage()
{
    constructImage();
    m_shapeController->setImage(m_image);
    m_nodeModel->setDummiesFacade(m_shapeController, m_image, 0, 0, 0);
    new ModelTest(m_nodeModel, this);
}

void KisNodeModelTest::testAddNode()
{
    m_shapeController->setImage(m_image);
    m_nodeModel->setDummiesFacade(m_shapeController, m_image, 0, 0, 0);
    new ModelTest(m_nodeModel, this);

    constructImage();

}

void KisNodeModelTest::testRemoveAllNodes()
{
    constructImage();
    m_shapeController->setImage(m_image);
    m_nodeModel->setDummiesFacade(m_shapeController, m_image, 0, 0, 0);
    new ModelTest(m_nodeModel, this);

    m_image->removeNode(m_layer4);
    m_image->removeNode(m_layer3);
    m_image->removeNode(m_layer2);
    m_image->removeNode(m_layer1);
}

void KisNodeModelTest::testRemoveIncludingRoot()
{
    constructImage();
    m_shapeController->setImage(m_image);
    m_nodeModel->setDummiesFacade(m_shapeController, m_image, 0, 0, 0);
    new ModelTest(m_nodeModel, this);

    m_image->removeNode(m_layer4);
    m_image->removeNode(m_layer3);
    m_image->removeNode(m_layer2);
    m_image->removeNode(m_layer1);
    m_image->removeNode(m_image->root());


}

void KisNodeModelTest::testSubstituteRootNode()
{
    constructImage();
    m_shapeController->setImage(m_image);
    m_nodeModel->setDummiesFacade(m_shapeController, m_image, 0, 0, 0);
    new ModelTest(m_nodeModel, this);

    m_image->flatten(0);
}

KISTEST_MAIN(KisNodeModelTest)
