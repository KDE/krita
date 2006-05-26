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

#ifndef KOOBJECTMANAGER_H
#define KOOBJECTMANAGER_H

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
 * The object manager hold a list of all object which are in scope.
 * There is one object manager per view.
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
     * Constructor that takes a list of objects, convenience version.
     * @param objects the objects to start out with, see also setObjects()
     * @param canvas the canvas this shape manager is working on.
     */
    KoShapeManager(KoCanvasBase *canvas, QList<KoShape *> &objects);
    virtual ~KoShapeManager();

    /**
     * Remove all previously owned objects and make the argument list the new objects
     * to be managed by this manager.
     * @param objects the new objects to manage.
     */
    void setObjects( QList<KoShape *> &objects );
    /// returns the list of maintained objects
    const QList<KoShape *> & objects() const { return m_objects; }

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

    /// return the selection object for this shapeManager
    KoSelection * selection() const { return m_selection; }

    /**
     * Paint all objects and their selection handles etc.
     * @param painter the painter to paint to.
     * @param forPrint if true, make sure only actual content is drawn and no decorations.
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paint( QPainter &painter, KoViewConverter &converter, bool forPrint );

    /**
     * Returns the object located at a specific point in the document.
     * @param position the position in the normal coordinate system.
     */
    KoShape * getObjectAt( const QPointF &position );

signals:
    /// emitted when the selection is changed
    void selectionChanged();

private:
    QList<KoShape *> m_objects;
    KoSelection * m_selection;
    KoRepaintManager *m_repaintManager;
};

#endif /* KOOBJECTMANAGER_H */

