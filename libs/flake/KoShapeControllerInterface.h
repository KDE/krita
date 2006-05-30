/* This file is part of the KDE project

   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSHAPECONTROLLERINTERFACE_H
#define KOSHAPECONTROLLERINTERFACE_H

/**
 * The shape controller is an abstract interface that the applications class
 * that controls the shapes should implement.  This tends to be the document.
 * @see KoShapeDeleteCommand, KoShapeCreateCommand
 */
class KoShapeControllerInterface
{
public:
    KoShapeControllerInterface() { }
    virtual ~KoShapeControllerInterface() { }

    /**
     * Add a shape to the shape controller, allowing it to be seen and saved.
     * The controller should add the shape to the ShapeManager instance(s) manually
     * if the shape is one that should be currently shown on screen.
     * @param shape the new shape
     */
    virtual void addShape( KoShape* shape ) = 0;

    /**
     * Remove a shape from the shape controllers control, allowing it to be deleted shortly after
     * The controller should remove the shape from all the ShapeManager instance(s) manually
     * @param shape the shape to remove
     */
    virtual void removeShape( KoShape* shape ) = 0;

    /**
     * Factory method to create shapes.
     * The CreateShapesTool will call this one when the user selected the outline for
     * the position and size of the shape.<br>
     * The implementor of this interface can choose to create different types of shapes
     * based on things like which button was pressed to activate the create tool.
     */
    virtual KoShape* createShape(QRectF outline) const = 0;
};

#endif // KOSHAPECONTROLLERINTERFACE_H
