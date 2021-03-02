/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "HistoryDock.h"
#include <KoDocumentResourceManager.h>
#include <kis_config.h>
#include <kis_icon_utils.h>

#include <QDebug>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>

#include <DlgConfigureHistoryDock.h>

HistoryDock::HistoryDock()
    : QDockWidget()
    , m_historyCanvas(0)
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *vl = new QVBoxLayout(page);
    m_undoView = new KisUndoView(this);
    vl->addWidget(m_undoView);
    QHBoxLayout *hl = new QHBoxLayout();
    hl->addSpacerItem(new QSpacerItem(10, 1,  QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_bnConfigure = new QToolButton(page);
    m_bnConfigure->setIcon(KisIconUtils::loadIcon("configure-thicker"));
    m_bnConfigure->setAutoRaise(true);
    connect(m_bnConfigure, SIGNAL(clicked(bool)), SLOT(configure()));
    hl->addWidget(m_bnConfigure);
    vl->addLayout(hl);

    setWidget(page);
    setWindowTitle(i18n("Undo History"));
}

void HistoryDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
    QPointer<KisCanvas2> myCanvas = dynamic_cast<KisCanvas2*>(canvas);
    if (myCanvas
            && myCanvas->shapeController()
            && myCanvas->shapeController()->resourceManager()
            && myCanvas->shapeController()->resourceManager()->undoStack()) {
        KUndo2Stack* undoStack = myCanvas->shapeController()->resourceManager()->undoStack();

        m_undoView->setStack(undoStack);
        KisConfig cfg(true);
        m_undoView->stack()->setUseCumulativeUndoRedo(cfg.useCumulativeUndoRedo());
        m_undoView->stack()->setTimeT1(cfg.stackT1());
        m_undoView->stack()->setTimeT2(cfg.stackT2());
        m_undoView->stack()->setStrokesN(cfg.stackN());
    }
    m_undoView->setCanvas( myCanvas );

}
void HistoryDock::configure()
{
    DlgConfigureHistoryDock dlg(m_undoView, m_undoView->stack(), this);
    dlg.exec();
}

void HistoryDock::unsetCanvas()
{
    m_historyCanvas = 0;
    setEnabled(false);
    m_undoView->setStack(0);
}
