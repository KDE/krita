/* This file is part of the KDE project

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

#ifndef KOSHAPEMANAGER_H
#define KOSHAPEMANAGER_H

#include <QList>
#include <QObject>

#include <koffice_export.h>

class KoShape;
class KoSelection;
class KoRepaintManager;
class KoViewConverter;
class KoCanvasBase;

class QPainter;
class QPointF;

/**
 * The shape manager hold a list of all shape which are in scope.
 * There is one shape manager per view.
 *
 * The selection in the different views can be different.
 */
class FLAKE_EXPORT KoShapeManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Empty constructor.
     */
    KoShapeManager(KoCanvasBase *canvas);
    /**
     * Constructor that takes a list of shapes, convenience version.
     * @param shapes the shapes to start out with, see also setShapes()
     * @param canvas the canvas this shape manager is working on.
     */
    KoShapeManager(KoCanvasBase *canvas, const QList<KoShape *> &shapes);
    virtual ~KoShapeManager();

    /**
     * Remove all previously owned shapes and make the argument list the new shapes
     * to be managed by this manager.
     * @param shapes the new shapes to manage.
     */
    void setShapes( const QList<KoShape *> &shapes );
    /// returns the list of maintained shapes
    const QList<KoShape *> & shapes() const { return m_shapes; }

    /**
     * Add a KoShape to be displayed and managed by this manager.
     * @param shape the shape to add
     */
    void add(KoShape *shape);
    /**
     * Remove a KoShape from this manager
     * @param shape the shape to remove
     */
    void remove(KoShape *shape);

    /// return the selection shapes for this shapeManager
    KoSelection * selection() const { return m_selection; }

    /**
     * Paint all shapes and their selection handles etc.
     * @param painter the painter to paint to.
     * @param forPrint if true, make sure only actual content is drawn and no decorations.
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paint( QPainter &painter, KoViewConverter &converter, bool forPrint );

    /**
     * Returns the shape located at a specific point in the document.
     * @param position the position in the normal coordinate system.
     */
    KoShape * shapeAt( const QPointF &position );

signals:
    /// emitted when the selection is changed
    void selectionChanged();

private:
    QList<KoShape *> m_shapes;
    KoSelection * m_selection;
    KoRepaintManager *m_repaintManager;
};

#endif

