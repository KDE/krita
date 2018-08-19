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

#include "KisPart.h"
#include "KisDocument.h"
#include "kis_name_server.h"
#include "flake/kis_shape_controller.h"

#include <sdk/tests/testutil.h>

KisShapeControllerTest::~KisShapeControllerTest()
{
}

KisDummiesFacadeBase* KisShapeControllerTest::dummiesFacadeFactory()
{

    m_doc = KisPart::instance()->createDocument();

    m_nameServer = new KisNameServer();
    return new KisShapeController(m_doc, m_nameServer);
}

void KisShapeControllerTest::destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade)
{
    delete dummiesFacade;
    delete m_nameServer;
    delete m_doc;
}

KISTEST_MAIN(KisShapeControllerTest)
