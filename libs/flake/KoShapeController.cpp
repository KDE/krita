/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoShapeController.h"
#include "KoShapeControllerBase.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"
#include "KoShapeLayer.h"
#include "KoSelection.h"
#include "commands/KoShapeCreateCommand.h"
#include "commands/KoShapeDeleteCommand.h"
#include "KoCanvasBase.h"
#include "KoShapeConfigWidgetBase.h"
#include "KoShapeConfigFactory.h"
#include "KoShapeFactory.h"
#include "KoShape.h"

#include <kpagedialog.h>
#include <klocale.h>

KoShapeController::KoShapeController( KoCanvasBase *canvas, KoShapeControllerBase *shapeController )
: m_canvas( canvas )
, m_shapeController( shapeController )
{
}

QUndoCommand* KoShapeController::addShape( KoShape *shape )
{
    Q_ASSERT(m_canvas->shapeManager());

    KoShapeFactory *factory = KoShapeRegistry::instance()->get( shape->shapeId() );
    Q_ASSERT(factory);
    int z=0;
    foreach(KoShape *sh, m_canvas->shapeManager()->shapesAt(shape->boundingRect()))
        z = qMax(z, sh->zIndex());
    shape->setZIndex(z+1);

    // show config dialog.
    KPageDialog *dialog = new KPageDialog(m_canvas->canvasWidget());
    dialog->setCaption(i18n("%1 Options", factory->name()));

    int pageCount = 0;
    QList<KoShapeConfigFactory*> panels = factory->panelFactories();
    qSort(panels.begin(), panels.end(), KoShapeConfigFactory::compare);
    QList<KoShapeConfigWidgetBase*> widgets;
    foreach (KoShapeConfigFactory *panelFactory, panels) {
        if(! panelFactory->showForShapeId( shape->shapeId() ) )
            continue;
        KoShapeConfigWidgetBase *widget = panelFactory->createConfigWidget(shape);
        if(widget == 0)
            continue;
        widgets.append(widget);
        widget->setUnit(m_canvas->unit());
        dialog->addPage(widget, panelFactory->name());
        pageCount ++;
    }
    foreach(KoShapeConfigWidgetBase* panel, factory->createShapeOptionPanels()) {
        panel->open(shape);
        widgets.append(panel);
        panel->setUnit(m_canvas->unit());
        dialog->addPage(panel, panel->objectName());
        pageCount ++;
    }

    if(pageCount > 0) {
        if(pageCount > 1)
            dialog->setFaceType(KPageDialog::Tabbed);
        if(dialog->exec() != KPageDialog::Accepted) {
            delete dialog;
            return 0;
        }
        foreach(KoShapeConfigWidgetBase *widget, widgets) {
            widget->save();
            // TODO action;
        }
    }
    delete dialog;
    // set the active layer as parent if there is not yet a parent.
    if ( !shape->parent() )
    {
        shape->setParent( m_canvas->shapeManager()->selection()->activeLayer() );
    }

    KoShapeCreateCommand *cmd = new KoShapeCreateCommand( m_shapeController, shape );
    return cmd;
}

QUndoCommand* KoShapeController::removeShape( KoShape *shape )
{
    KoShapeDeleteCommand *cmd = new KoShapeDeleteCommand( m_shapeController, shape );
    return cmd;
}

QUndoCommand* KoShapeController::removeShapes( const QList<KoShape*> &shapes) {
    return new KoShapeDeleteCommand( m_shapeController, shapes );
}
