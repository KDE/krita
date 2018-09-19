
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

#include "kis_node_model_test.h"

#include <QTest>
#include <kis_debug.h>

#include "KisDocument.h"
#include "KisPart.h"
#include "kis_node_model.h"
#include "kis_name_server.h"
#include "flake/kis_shape_controller.h"

#include <sdk/tests/testutil.h>

#include "modeltest.h"

void KisNodeModelTest::init()
{
    m_doc = KisPart::instance()->createDocument();

    m_nameServer = new KisNameServer();
    m_shapeController = new KisShapeController(m_doc, m_nameServer);
    m_nodeModel = new KisNodeModel(0);

    initBase();
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
