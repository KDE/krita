/*
    Copyright (C) 2012  Dan Leinir Turthra Jensen <admin@leinir.dk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

