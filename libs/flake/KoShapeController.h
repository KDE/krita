/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include <koffice_export.h>

#include <kcommand.h>

#include <QString>

class KoCanvasBase;
class KoShape;
class KoShapeControllerBase;
class KoProperties;

/**
 * Class used by tools to maintain the list of shapes.
 * All applications have some sort of list of all shapes that belong to the document.
 * The applications implement the KoShapeControllerBase interface (all pure virtuals)
 * to add and remove shapes from the document.  To ensure that an application can expect
 * a certain protocol to be adhered to when adding/removing shapes, all tools use the API
 * from this class for maintaining the list of shapes in the document.  So no tool gets
 * to acess the application directly.
 */
class FLAKE_EXPORT KoShapeController {
public:
    /**
     * Create a new Controller; typically not called by applications, only by the KoToolManager
     * @param canvas the canvas this controller works for.
     */
    KoShapeController(KoCanvasBase *canvas);
    /// destructor
    ~KoShapeController() {};

    /**
     * Set the shape controller that this controller adds its created shapes to.
     * This controller will create a shape after user input and that shape has to be
     * registered to the hosting application.  We add/remove the shape via the
     * KoShapeControllerBase interface.<br>
     * Note that this controller will create a command that can be used for undo/redo.
     * The undo will actually call the remove.
     * @param sc the controller that will be used to add/remove the created shape.
     */
    void setShapeController(KoShapeControllerBase *sc) { m_shapeController = sc; }

    /**
     * Each shape-type has an Id; as found in KoShapeFactory::id().id(), to choose which
     * shape this controller should actually create; set the id before the user starts to
     * create the new shape.
     * @param id the SHAPEID of the to be generated shape
     */
    void setShapeId(const QString &id) { m_shapeId = id; }
    /**
     * return the shape Id that is to be created.
     * @return the shape Id that is to be created.
     */
    const QString &shapeId() const { return m_shapeId; }

    /**
     * Set the shape properties that the create controller will use for the next shape it will
     * create.
     * @param properties the properties or 0 if the default shape should be created.
     */
    void setShapeProperties(KoProperties *properties) { m_newShapeProperties = properties; }
    /**
     * return the properties to be used for creating the next shape
     * @return the properties to be used for creating the next shape
     */
    KoProperties const * shapeProperties() { return m_newShapeProperties; }

    /// Suggested API for deleting a shape
    KCommand* deleteShape( KoShape *shape);

    /// Suggested API for creating a new shape, and registring it with the hosting application.
    KCommand* createShape( KoShape *shape );

protected:
    friend class KoCreateShapeStrategy;
    /**
     * Returns the shape Controller that is registered to hold the shapes this class creates.
     * @return the shape controller.
     */
    KoShapeControllerBase* controller() const { return m_shapeController; }

private:
    KoShapeControllerBase *m_shapeController;
    QString m_shapeId;
    KoProperties *m_newShapeProperties;
    KoCanvasBase *m_canvas;
};

#endif
