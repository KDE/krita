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

#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include "KoColorSpace.h"
#include "KoCompositeOp.h"
#include "KoColorSpaceRegistry.h"

#include "kis_doc2.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer_model.h"
#include "kis_nameserver.h"
#include "kis_paint_layer.h"
#include "kis_shape_controller.h"
#include "kis_types.h"

#include "kis_shape_controller_test.h"

void KisShapeControllerTest::initTestCase ()
{
    m_doc = new KisDoc2();
    m_nameServer = new KisNameServer();
    m_shapeController = new KisShapeController(m_doc, m_nameServer);

    KoColorSpaceRegistry * reg = KoColorSpaceRegistry::instance();
    KoColorSpace * colorSpace = reg->colorSpace("RGBA", 0);

    m_image = new KisImage(0, 512, 512, colorSpace, "shape controller test");
//  m_shapeController->setImage( m_image );
}

void KisShapeControllerTest::cleanupTestCase ()
{
//     m_shapeController->setImage( 0 );
    delete m_image;
    delete m_shapeController;
    delete m_nameServer;
    delete m_doc;
}


void KisShapeControllerTest::testSetImage()
{
}

void KisShapeControllerTest::testAddShape()
{

}

void KisShapeControllerTest::testRemoveShape()
{
}

void KisShapeControllerTest::testSetInitialShapeForView()
{
}

void KisShapeControllerTest::testShapeForLayer()
{
}


QTEST_KDEMAIN(KisShapeControllerTest, GUI)
#include "kis_shape_controller_test.moc"
