/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestNodeView.h"

#include <QTest>
#include <QDialog>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include <NodeView.h>

#include "KisDocument.h"
#include "KisPart.h"
#include "kis_name_server.h"
#include "flake/kis_shape_controller.h"
#include "kis_undo_adapter.h"
#include "kis_node_model.h"
#include "kis_color_filter_combo.h"

#include <sdk/tests/testutil.h>

//#define ENABLE_GUI_TESTS


void NodeViewTest::init()
{
    m_doc = KisPart::instance()->createDocument();

    m_nameServer = new KisNameServer();
    m_shapeController = new KisShapeController(m_nameServer, m_doc->undoStack());

    initBase();
}

void NodeViewTest::cleanup()
{
    cleanupBase();

    delete m_shapeController;
    delete m_nameServer;
    delete m_doc;
}


void NodeViewTest::testLayers()
{
#ifndef ENABLE_GUI_TESTS
    return;
#endif

    QDialog dlg;

    QFont font;
    font.setPointSizeF(8);
    dlg.setFont(font);

    KisNodeModel *model = new KisNodeModel(this);
    NodeView *view = new NodeView(&dlg);

    view->setModel(model);

    constructImage();
    addSelectionMasks();
    m_shapeController->setImage(m_image);

    model->setDummiesFacade(m_shapeController, m_image, m_shapeController, 0, 0, 0, 0);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    KisColorFilterCombo *cb = new KisColorFilterCombo(&dlg);

    QSet<int> labels;
    for (int i = 0; i < 6; i++) {
        labels.insert(i);
    }
    cb->updateAvailableLabels(labels);

    QHBoxLayout *hbox = new QHBoxLayout(&dlg);
    hbox->addStretch(1);
    hbox->addWidget(cb);
    layout->addLayout(hbox);
    layout->addWidget(view);

    dlg.resize(280, 400);
    view->expandAll();

    dlg.exec();
}

#include "kis_color_label_selector_widget.h"

void NodeViewTest::testColorLabels()
{
#ifndef ENABLE_GUI_TESTS
    return;
#endif


    QDialog dlg;

    QFont font;
    font.setPointSizeF(8);
    dlg.setFont(font);

    KisColorLabelSelectorWidget *widget = new KisColorLabelSelectorWidget(&dlg);
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    widget->setSizePolicy(policy);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->addWidget(widget);
    layout->addStretch(1);

    dlg.resize(280, 400);
    dlg.exec();
}

KISTEST_MAIN(NodeViewTest)
