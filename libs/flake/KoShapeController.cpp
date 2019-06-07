/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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
#include "KoDocumentResourceManager.h"
#include "KoShapeManager.h"
#include "KoShapeLayer.h"
#include "KoSelection.h"
#include "commands/KoShapeCreateCommand.h"
#include "commands/KoShapeDeleteCommand.h"
#include "commands/KoShapeConnectionChangeCommand.h"
#include "KoCanvasBase.h"
#include "KoShapeConfigWidgetBase.h"
#include "KoShapeFactoryBase.h"
#include "KoShape.h"
#include "KoConnectionShape.h"
#include <KoUnit.h>

#include <QObject>

#include <kpagedialog.h>
#include <klocalizedstring.h>

class KoShapeController::Private
{
public:
    Private()
        : canvas(0),
          shapeController(0)
    {
    }

    KoCanvasBase *canvas;
    KoShapeControllerBase *shapeController;

    KUndo2Command* addShape(KoShape *shape, bool showDialog, KoShapeContainer *parentShape, KUndo2Command *parent) {

        if (canvas) {
            if (showDialog && !shape->shapeId().isEmpty()) {
                KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value(shape->shapeId());
                Q_ASSERT(factory);
                qint16 z = 0;
                Q_FOREACH (KoShape *sh, canvas->shapeManager()->shapes()) {
                    z = qMax(z, sh->zIndex());
                }
                shape->setZIndex(z + 1);

                // show config dialog.
                KPageDialog *dialog = new KPageDialog(canvas->canvasWidget());
                dialog->setWindowTitle(i18n("%1 Options", factory->name()));

                int pageCount = 0;
                QList<KoShapeConfigWidgetBase*> widgets;
                Q_FOREACH (KoShapeConfigWidgetBase* panel, factory->createShapeOptionPanels()) {
                    if (! panel->showOnShapeCreate())
                        continue;
                    panel->open(shape);
                    panel->connect(panel, SIGNAL(accept()), dialog, SLOT(accept()));
                    widgets.append(panel);
                    panel->setResourceManager(canvas->resourceManager());
                    panel->setUnit(canvas->unit());
                    QString title = panel->windowTitle().isEmpty() ? panel->objectName() : panel->windowTitle();
                    dialog->addPage(panel, title);
                    pageCount ++;
                }

                if (pageCount > 0) {
                    if (pageCount > 1)
                        dialog->setFaceType(KPageDialog::Tabbed);
                    if (dialog->exec() != KPageDialog::Accepted) {
                        delete dialog;
                        return 0;
                    }
                    Q_FOREACH (KoShapeConfigWidgetBase *widget, widgets)
                        widget->save();
                }
                delete dialog;
            }
        }

        return addShapesDirect({shape}, parentShape, parent);
    }

    KUndo2Command* addShapesDirect(const QList<KoShape*> shapes, KoShapeContainer *parentShape, KUndo2Command *parent)
    {
        return new KoShapeCreateCommand(shapeController, shapes, parentShape, parent);
    }

    void handleAttachedConnections(KoShape *shape, KUndo2Command *parentCmd) {
        foreach (KoShape *dependee, shape->dependees()) {
            KoConnectionShape *connection = dynamic_cast<KoConnectionShape*>(dependee);
            if (connection) {
                if (shape == connection->firstShape()) {
                    new KoShapeConnectionChangeCommand(connection, KoConnectionShape::StartHandle,
                                                       shape, connection->firstConnectionId(), 0, -1, parentCmd);
                } else if (shape == connection->secondShape()) {
                    new KoShapeConnectionChangeCommand(connection, KoConnectionShape::EndHandle,
                                                       shape, connection->secondConnectionId(), 0, -1, parentCmd);
                }
            }
        }
    }
};

KoShapeController::KoShapeController(KoCanvasBase *canvas, KoShapeControllerBase *shapeController)
    : d(new Private())
{
    d->canvas = canvas;
    d->shapeController = shapeController;
}

KoShapeController::~KoShapeController()
{
    delete d;
}

void KoShapeController::reset()
{
    d->canvas = 0;
    d->shapeController = 0;
}

KUndo2Command* KoShapeController::addShape(KoShape *shape, KoShapeContainer *parentShape, KUndo2Command *parent)
{
    return d->addShape(shape, true, parentShape, parent);
}

KUndo2Command* KoShapeController::addShapeDirect(KoShape *shape, KoShapeContainer *parentShape, KUndo2Command *parent)
{
    return d->addShapesDirect({shape}, parentShape, parent);
}

KUndo2Command *KoShapeController::addShapesDirect(const QList<KoShape *> shapes, KoShapeContainer *parentShape, KUndo2Command *parent)
{
    return d->addShapesDirect(shapes, parentShape, parent);
}

KUndo2Command* KoShapeController::removeShape(KoShape *shape, KUndo2Command *parent)
{
    KUndo2Command *cmd = new KoShapeDeleteCommand(d->shapeController, shape, parent);
    QList<KoShape*> shapes;
    shapes.append(shape);
    d->shapeController->shapesRemoved(shapes, cmd);
    // detach shape from any attached connection shapes
    d->handleAttachedConnections(shape, cmd);
    return cmd;
}

KUndo2Command* KoShapeController::removeShapes(const QList<KoShape*> &shapes, KUndo2Command *parent)
{
    KUndo2Command *cmd = new KoShapeDeleteCommand(d->shapeController, shapes, parent);
    d->shapeController->shapesRemoved(shapes, cmd);
    foreach (KoShape *shape, shapes) {
        d->handleAttachedConnections(shape, cmd);
    }
    return cmd;
}

void KoShapeController::setShapeControllerBase(KoShapeControllerBase *shapeController)
{
    d->shapeController = shapeController;
}

QRectF KoShapeController::documentRectInPixels() const
{
    return d->shapeController ? d->shapeController->documentRectInPixels() : QRectF(0,0,1920,1080);
}

qreal KoShapeController::pixelsPerInch() const
{
    return d->shapeController ? d->shapeController->pixelsPerInch() : 72.0;
}

QRectF KoShapeController::documentRect() const
{
    return d->shapeController ? d->shapeController->documentRect() : documentRectInPixels();
}

KoDocumentResourceManager *KoShapeController::resourceManager() const
{
    if (!d->shapeController) {
        qWarning() << "THIS IS NOT GOOD!";
        return 0;
    }
    return d->shapeController->resourceManager();
}

KoShapeControllerBase *KoShapeController::documentBase() const
{
    return d->shapeController;
}
