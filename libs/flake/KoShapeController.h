/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSHAPECONTROLLER_H
#define KOSHAPECONTROLLER_H

#include "kritaflake_export.h"

#include <QObject>
#include <QList>
#include <QMetaType>

class KoCanvasBase;
class KoShape;
class KoShapeContainer;
class KoShapeControllerBase;
class KUndo2Command;
class KoDocumentResourceManager;

/**
 * Class used by tools to maintain the list of shapes.
 * All applications have some sort of list of all shapes that belong to the document.
 * The applications implement the KoShapeControllerBase interface (all pure virtuals)
 * to add and remove shapes from the document. To ensure that an application can expect
 * a certain protocol to be adhered to when adding/removing shapes, all tools use the API
 * from this class for maintaining the list of shapes in the document. So no tool gets
 * to access the application directly.
 */
class KRITAFLAKE_EXPORT KoShapeController : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a new Controller; typically not called by applications, only
     * by the KonCanvasBase constructor.
     * @param canvas the canvas this controller works for. The canvas can be 0
     * @param shapeController the application provided shapeController that we can call.
     */
    KoShapeController(KoCanvasBase *canvas, KoShapeControllerBase *shapeController);
    /// destructor
    ~KoShapeController() override;

    /**
     * @brief reset sets the canvas and shapebased document to 0.
     */
    void reset();

    /**
     * @brief Add a shape to the document.
     * If the shape has no parent, the active layer will become its parent.
     *
     * @param shape to add to the document
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will insert the shape into the document or 0 if the
     *         insertion was cancelled. The command is not yet executed.
     */
    KUndo2Command* addShape(KoShape *shape, KoShapeContainer *parentShape, KUndo2Command *parent = 0);

    /**
     * @brief Add a shape to the document, skipping any dialogs or other user interaction.
     *
     * @param shape to add to the document
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will insert the shape into the document. The command is not yet executed.
     */
    KUndo2Command* addShapeDirect(KoShape *shape, KoShapeContainer *parentShape, KUndo2Command *parent = 0);

    /**
     * @brief Add shapes to the document, skipping any dialogs or other user interaction.
     *
     * @param shape  the shape to add to the document
     * @param parentShape the parent shape
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will insert the shapes into the document. The command is not yet executed.
     */
    KUndo2Command* addShapesDirect(const QList<KoShape*> shape, KoShapeContainer *parentShape, KUndo2Command *parent = 0);

    /**
     * @brief Remove a shape from the document.
     *
     * @param shape to remove from the document
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will remove the shape from the document.
     *         The command is not yet executed.
     */
    KUndo2Command* removeShape(KoShape *shape, KUndo2Command *parent = 0);

    /**
     * Remove a shape from the document.
     *
     * @param shapes the set of shapes to remove from the document
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will remove the shape from the document.
     *         The command is not yet executed.
     */
    KUndo2Command* removeShapes(const QList<KoShape*> &shapes, KUndo2Command *parent = 0);

    /**
     * @brief Set the KoShapeControllerBase used to add/remove shapes.
     *
     * NOTE: only Sheets uses this method. Do not use it in your application. Sheets
     * has to also call:
     * <code>KoToolManager::instance()->updateShapeControllerBase(shapeController, canvas->canvasController());</code>
     *
     * @param shapeController the new shapeController.
     */
    void setShapeControllerBase(KoShapeControllerBase *shapeController);

    /**
     * The size of the document measured in rasterized pixels. This information is needed for loading
     * SVG documents that use 'px' as the default unit.
     */
    QRectF documentRectInPixels() const;

    /**
     * Resolution of the rasterized representation of the document. Used to load SVG documents correctly.
     */
    qreal pixelsPerInch() const;

    /**
     * Document rect measured in 'pt'
     */
    QRectF documentRect() const;

    /**
     * Return a pointer to the resource manager associated with the
     * shape-set (typically a document). The resource manager contains
     * document wide resources * such as variable managers, the image
     * collection and others.
     */
    KoDocumentResourceManager *resourceManager() const;

    /**
     * @brief Returns the KoShapeControllerBase used to add/remove shapes.
     *
     * @return the KoShapeControllerBase
     */
    KoShapeControllerBase *documentBase() const;

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoShapeController *)

#endif
