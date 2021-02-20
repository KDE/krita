/*
    SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "ToolManager.h"
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_tool.h>

#include <KoToolManager.h>

class ToolManager::Private
{
public:
    Private()
        : view(0)
        , currentTool(0)
    {
        toolManager = KoToolManager::instance();
    };

    KoToolManager* toolManager;
    KisViewManager* view;
    KisTool* currentTool;
};

ToolManager::ToolManager(QQuickItem* parent)
    : QQuickItem(parent)
    , d(new Private)
{
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            this, SLOT(slotToolChanged(KoCanvasController*,int)));
}

ToolManager::~ToolManager()
{
    delete d;
}

QObject* ToolManager::view() const
{
    return d->view;
}

void ToolManager::setView(QObject* newView)
{
    d->view = qobject_cast<KisViewManager*>( newView );
    slotToolChanged(0, 0);
    emit viewChanged();
}

void ToolManager::requestToolChange(QString toolID)
{
    if (d->view) {
        d->toolManager->switchToolRequested(toolID);
    }
}

QObject* ToolManager::currentTool() const
{
    return d->currentTool;
}

void ToolManager::slotToolChanged(KoCanvasController* canvas, int toolId)
{
    Q_UNUSED(canvas);
    Q_UNUSED(toolId);

    if (!d->view) return;
    if (!d->view->canvasBase()) return;


    QString  id   = KoToolManager::instance()->activeToolId();
    d->currentTool = dynamic_cast<KisTool*>(KoToolManager::instance()->toolById(d->view->canvasBase(), id));
    emit currentToolChanged();
}

