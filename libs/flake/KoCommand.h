/* This file is part of the KDE project
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

#ifndef KOCommand_h
#define KOCommand_h

#include <kcommand.h>
#include <koffice_export.h>

#include <KoSelection.h>

#include <QList>
#include <QPointF>

class KoShape;
class KoShapeGroup;
class KoShapeContainer;
class KoShapeControllerInterface;
class QString;

/// The undo / redo command for shape moving.
class FLAKE_EXPORT KoShapeMoveCommand : public KCommand {
public:
    /**
     * Constructor.
     * @param shapes the set of objects that are moved.
     * @param previousPositions the known set of previous positions for each of the objects.
     *  this list naturally must have the same amount of items as the shapes set.
     * @param newPositions the new positions for the shapes.
     *  this list naturally must have the same amount of items as the shapes set.
     */
    KoShapeMoveCommand(const KoSelectionSet &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions);
    void execute ();
    void unexecute ();
    QString name () const;

private:
    QList<KoShape*> m_shapes;
    QList<QPointF> m_previousPositions, m_newPositions;
};

/// The undo / redo command for shape rotating.
class FLAKE_EXPORT KoShapeRotateCommand : public KCommand {
public:
    /**
     * Comand to rotate a selection of shapes.  Note that it just alters the rotated
     * property of those shapes, and nothing more.
     * @param shapes all the shapes that should be rotated
     * @param previousAngles a list with the same amount of items as shapes with the
     *        old rotation angles
     * @param newAngles a list with the same amount of items as shapes with the new angles.
     */
    KoShapeRotateCommand(const KoSelectionSet &shapes, QList<double> &previousAngles, QList<double> &newAngles);
    void execute ();
    void unexecute ();
    QString name () const;
private:
    QList<KoShape*> m_shapes;
    QList<double> m_previousAngles, m_newAngles;
};

/// The undo / redo command for shape sizing.
class FLAKE_EXPORT KoShapeSizeCommand : public KCommand {
public:
    KoShapeSizeCommand(const KoSelectionSet &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes);
    void execute ();
    void unexecute ();
    QString name () const;
private:
    QList<KoShape*> m_shapes;
    QList<QSizeF> m_previousSizes, m_newSizes;
};

/// The undo / redo command for grouping shapes
class FLAKE_EXPORT KoGroupShapesCommand : public KCommand {
public:
    /**
     * Command to group a set of shapes into a predefined container.
     * @param container the container to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     * @param clipped a list of the same length as the shapes list with one bool for each shape.
     *      See KoShapeContainer::childClipped()
     */
    KoGroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped);
    /**
     * Command to group a set of shapes into a predefined container.
     * Convenience constructor since KoShapeGroup does not allow clipping.
     * @param container the group to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     */
    KoGroupShapesCommand(KoShapeGroup *container, QList<KoShape *> shapes);
    virtual ~KoGroupShapesCommand() { };
    virtual void execute ();
    virtual void unexecute ();
    virtual QString name () const;

protected:
    KoGroupShapesCommand(); ///< protected constructor for child classes
    QList<KoShape*> m_shapes; ///<list of shapes to be grouped
    QList<bool> m_clipped; ///< list of booleas to specify the shape of the same index to eb clipped
    KoShapeContainer *m_container; ///< the container where the grouping should be for.
};

/// The undo / redo command for ungrouping shapes
class FLAKE_EXPORT KoUngroupShapesCommand : public KoGroupShapesCommand {
public:
    /**
     * Command to ungroup a set of shapes from one parent container.
     * @param container the group to ungroup the shapes from.
     * @param shapes a list of all the shapes that should be ungrouped.
     */
    KoUngroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes);
    void execute ();
    void unexecute ();
    QString name () const;
};

/// The undo / redo command for creating shapes
class FLAKE_EXPORT KoShapeCreateCommand : public KCommand {
public:
    KoShapeCreateCommand( KoShapeControllerInterface *controller, KoShape *shape );
    virtual ~KoShapeCreateCommand();
    void execute ();
    void unexecute ();
    virtual QString name () const;
protected:
    KoShapeControllerInterface *m_controller;
    KoShape *m_shape;
    bool m_deleteShape;
};

/// The undo / redo command for deleting shapes
class FLAKE_EXPORT KoShapeDeleteCommand : public KCommand {
public:
    /**
     * Command to delete a single shape by means of a shape controller.
     * @param controller the controller to used for deleting.
     * @param shape a single shape that should be deleted.
     */
    KoShapeDeleteCommand( KoShapeControllerInterface *controller, KoShape *shape );
    /**
     * Command to delete a set of shapes by means of a shape controller.
     * @param controller the controller to used for deleting.
     * @param shapes a set of all the shapes that should be deleted.
     */
    KoShapeDeleteCommand( KoShapeControllerInterface *controller, const KoSelectionSet &shapes );
    virtual ~KoShapeDeleteCommand();
    void execute ();
    void unexecute ();
    virtual QString name () const;
protected:
    KoShapeControllerInterface *m_controller;
    QList<KoShape*> m_shapes;
    bool m_deleteShapes;
};

#endif
