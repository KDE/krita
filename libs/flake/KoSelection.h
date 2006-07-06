/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
#include <QSet>

#include "KoShape.h"
#include "KoViewConverter.h"
#include "KoFlake.h"

#include <koffice_export.h>

typedef QSet<KoShape*> KoSelectionSet;

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
class FLAKE_EXPORT KoSelection : public QObject, public KoShape {
    Q_OBJECT

public:

    KoSelection();
    virtual ~KoSelection();

    virtual void paint( QPainter &painter, KoViewConverter &converter );

    /// add a selected object
    virtual void select(KoShape * object);
    /// remove a selected object
    virtual void deselect(KoShape * object);
    /// clear the selections list
    virtual void deselectAll();
    /**
     * Return the list of selected shapes
     * @return the list of selected shapes
     * @param strip if StrippedSelection, the returned list will not include any children
     *    of a grouped shape if the group-parent is itself also in the set.
     */
    virtual const KoSelectionSet selectedShapes(KoFlake::SelectionType strip = KoFlake::FullSelection) const;
    /**
     * Return the first selected shape, or 0 if there is nothing selected.
     * @param strip if StrippedSelection, the returned list will not include any children
     *    of a grouped shape if the group-parent is itself also in the set.
     */
    KoShape *firstSelectedShape(KoFlake::SelectionType strip = KoFlake::FullSelection) const;
    /// return if the shape is selected
    virtual bool isSelected(const KoShape *object) const;
    /// return the selection count
    virtual int count() const;

    virtual bool hitTest( const QPointF &position ) const;

    virtual QRectF boundingRect() const;

    /* Returns the size as from the time where count was last changed.
     * You need to apply the transformationmatrix if you have transformed the selection since then
     * This function is used in drawing the rotated/scale/moved outline of the selection.
     * Whenever a shape is added or removed to the selection the unmodifiedBoundingRect is reset, as
     * are the transformation matrix
     * Remember though that the transformation matrix is not used for anything else
     */
    QSizeF unmodifiedSize() const;

protected:    
    virtual void updateTree() {}

signals:
    /// emitted when the selection is changed
    void selectionChanged();

private slots:
    void selectionChangedEvent();

private:
    void requestSelectionChangedEvent();

    KoSelectionSet m_selectedObjects;
    bool m_eventTriggered;
    QSizeF m_unmodifiedSize;
};

#endif
