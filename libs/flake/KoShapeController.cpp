/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006-2008 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include "KoCanvasBase.h"
#include "KoShapeConfigWidgetBase.h"
#include "KoShapeFactoryBase.h"
#include "KoShape.h"
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
        KUndo2Command *resultCommand = 0;

        if (!parentShape) {
            resultCommand = new KUndo2Command(parent);
            parentShape = shapeController->createParentForShapes(shapes, resultCommand);
            KUndo2Command *addShapeCommand = new KoShapeCreateCommand(shapeController, shapes, parentShape, resultCommand);
            resultCommand->setText(addShapeCommand->text());
        } else {
            resultCommand = new KoShapeCreateCommand(shapeController, shapes, parentShape, parent);
        }

        return resultCommand;
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
    return removeShapes({shape}, parent);
}

KUndo2Command* KoShapeController::removeShapes(const QList<KoShape*> &shapes, KUndo2Command *parent)
{
    KUndo2Command *cmd = new KoShapeDeleteCommand(d->shapeController, shapes, parent);
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
