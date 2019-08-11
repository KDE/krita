/* This file is part of the KDE project
 * Copyright (C) 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    m_bnConfigure->setIcon(KisIconUtils::loadIcon("configure"));
    connect(m_bnConfigure, SIGNAL(clicked(bool)), SLOT(configure()));
    hl->addWidget(m_bnConfigure);
    vl->addItem(hl);
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
        KUndo2Stack* undoStack = canvas->shapeController()->resourceManager()->undoStack();

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
