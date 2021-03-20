/*
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shape_controller_test.h"

#include <simpletest.h>

#include "KisPart.h"
#include "KisDocument.h"
#include "kis_name_server.h"
#include "flake/kis_shape_controller.h"

#include <sdk/tests/testutil.h>
#include <sdk/tests/testui.h>

KisDummiesFacadeBase* KisShapeControllerTest::dummiesFacadeFactory()
{
    m_doc = KisPart::instance()->createDocument();

    m_nameServer = new KisNameServer();
    return new KisShapeController(m_nameServer, m_doc->undoStack());
}

void KisShapeControllerTest::destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade)
{
    delete dummiesFacade;
    delete m_nameServer;
    delete m_doc;
}

KISTEST_MAIN(KisShapeControllerTest)
