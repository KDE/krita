/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_view_test.h"

#include <QTest>
#include <QDialog>
#include <QVBoxLayout>

#include <KisNodeView.h>

#include "KisDocument.h"
#include "KisPart.h"
#include "kis_name_server.h"
#include "flake/kis_shape_controller.h"
#include "kis_undo_adapter.h"
#include "kis_node_model.h"


void KisNodeViewTest::init()
{
    m_doc = KisPart::instance()->createDocument();

    m_nameServer = new KisNameServer();
    m_shapeController = new KisShapeController(m_doc, m_nameServer);

    initBase();
}

void KisNodeViewTest::cleanup()
{
    cleanupBase();

    delete m_shapeController;
    delete m_nameServer;
    delete m_doc;
}


void KisNodeViewTest::test()
{
    QDialog dlg;

    QFont font;
    font.setPointSizeF(8);
    dlg.setFont(font);

    KisNodeModel *model = new KisNodeModel(this);
    KisNodeView *view = new KisNodeView(&dlg);

    view->setModel(model);

    constructImage();
    addSelectionMasks();
    m_shapeController->setImage(m_image);

    model->setDummiesFacade(m_shapeController, m_image, m_shapeController, 0, 0);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->addWidget(view);

    dlg.resize(280, 400);
    view->expandAll();

    dlg.exec();
}

QTEST_MAIN(KisNodeViewTest)
