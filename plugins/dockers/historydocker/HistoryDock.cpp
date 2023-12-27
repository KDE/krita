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

HistoryDock::HistoryDock()
    : QDockWidget()
    , m_undoView(new KisUndoView(this))
{
    setWidget(m_undoView);
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
    }
    m_undoView->setCanvas( myCanvas );

}

void HistoryDock::unsetCanvas()
{
    setEnabled(false);
    m_undoView->setStack(0);
}
