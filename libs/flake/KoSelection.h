/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007,2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2006,2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOSELECTION_H
#define KOSELECTION_H

#include <QObject>

#include "KoShape.h"
#include "KoFlake.h"

#include "kritaflake_export.h"

class KoShapeLayer;
class KoSelectionPrivate;

/**
 * A selection is a shape that contains a number of references
 * to shapes. That means that a selection can be manipulated in
 * the same way as a single shape.
 *
 * Note that a single shape can be selected in one view, and not in
 * another, and that in a single view, more than one selection can be
 * present. So selections should not be seen as singletons, or as
 * something completely transient.
 *
 * A selection, however, should not be selectable. We need to think
 * a little about the interaction here.
 */
class KRITAFLAKE_EXPORT KoSelection : public QObject, public KoShape, public KoShape::ShapeChangeListener
{
    Q_OBJECT

public:

    KoSelection(QObject *parent = 0);
    ~KoSelection() override;

    void paint(QPainter &painter, KoShapePaintingContext &paintcontext) override;
    void setSize(const QSizeF &size) override;
    QSizeF size() const override;
    QRectF outlineRect() const override;
    QRectF boundingRect() const override;

    /**
     * Adds a shape to the selection.
     *
     * If the shape is a KoShapeGroup all of its child shapes are automatically added
     * to the selection.
     * If the shape has no parent or is not a KoShapeGroup, only the given shape is
     * added to the selection.
     * If the given shape is a child of a KoShapeGroup and recursive selection is enabled
     * the all parents and their child shapes up to the toplevel KoShapeGroup are added to
     * the selection.
     *
     * @param shape the shape to add to the selection
     */
    void select(KoShape *shape);

    /**
     * Removes a selected shape.
     *
     * If the shape is a KoShapeGroup all of its child shapes are automatically removed
     * from the selection.
     * If the shape has no parent or is not a KoShapeGroup, only the given shape is
     * removed from the selection.
     * If the given shape is a child of a KoShapeGroup and recursive selection is enabled
     * the all parents and their child shape up to the toplevel KoShapeGroup are removed
     * from the selection.
     *
     * @param shape the shape to remove from the selection
     */
    void deselect(KoShape *shape);

    /// clear the selections list
    void deselectAll();

    /**
     * Return the list of selected shapes
     * @return the list of selected shapes
     */
    const QList<KoShape*> selectedShapes() const;

    /**
     * Same as selectedShapes() but only for shapes in visible state. Used by
     * the algorithms that draw shapes on the image
     */
    const QList<KoShape*> selectedVisibleShapes() const;

    /**
     * Same as selectedShapes() but only for editable shapes. Used by
     * the algorithms that modify the image
     */
    const QList<KoShape*> selectedEditableShapes() const;

    /**
     * Same as selectedEditableShapes() but also includes shapes delegates.
     * Used for
     */
    const QList<KoShape*> selectedEditableShapesAndDelegates() const;

    /**
     * Return the first selected shape, or 0 if there is nothing selected.
     */
    KoShape *firstSelectedShape() const;

    /// return true if the shape is selected
    bool isSelected(const KoShape *shape) const;

    /// return the selection count, i.e. the number of all selected shapes
    int count() const;

    bool hitTest(const QPointF &position) const override;

    /**
     * Sets the currently active layer.
     * @param layer the new active layer
     */
    void setActiveLayer(KoShapeLayer *layer);

    /**
     * Returns a currently active layer.
     *
     * @return the currently active layer, or zero if there is none
     */
    KoShapeLayer *activeLayer() const;

    void notifyShapeChanged(ChangeType type, KoShape *shape) override;

Q_SIGNALS:
    /// emitted when the selection is changed
    void selectionChanged();

    /// emitted when the current layer is changed
    void currentLayerChanged(const KoShapeLayer *layer);

private:
    void saveOdf(KoShapeSavingContext &) const override;
    bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) override;

protected:
    KoSelection(const KoSelection &rhs);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif
