/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>

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

#ifndef KTREE_LAYOUT_H
#define KTREE_LAYOUT_H

#include <QList>
#include <QMap>

#include <KoShapeContainerModel.h>

class QSizeF;
class KoConnectionShape;

class Layout : public KoShapeContainerModel
{
public:
    Layout(KoShapeContainer *container);
    ~Layout();

    /**
     * Adds a shape to the layout.
     */
    void add(KoShape *shape);

    /**
     * Adds a shape to the layout in the defined position.
     */
    void add(KoShape *shape, uint position);

    /**
     * Adds a root shape to the layout.
     */
    void setRoot(KoShape *shape);

    /**
     * Returns a pointer to root shape.
     */
    KoShape* root() const;

    /**
     * Attaches given connector to given shape.
     */
    void attachConnector(KoShape* shape, KoConnectionShape *connector);

    /**
     * Removes a shape from the layout.
     */
    void remove(KoShape *shape);

    /**
     * Returns a pointer to connector attached to given shape
     */
    KoShape* connector(KoShape *shape);

    /**
     * Turns clipping of a shape on or off.
     */
    void setClipped(const KoShape *shape, bool clipping);

    /**
     * @see setClipping
     */
    bool isClipped(const KoShape *shape) const;

    /// reimplemented
    virtual void setInheritsTransform(const KoShape *shape, bool inherit);
    /// reimplemented
    virtual bool inheritsTransform(const KoShape *shape) const;

    /**
     * Returns the number of shapes in this layout.
     */
    int count() const;

    /**
     * Returns a list of shapes in this layout.
     */
    QList<KoShape*> shapes() const;

    /**
     * Called whenever a property of the container (i.e. the Tree) is changed.
     */
    void containerChanged(KoShapeContainer *container, KoShape::ChangeType type);

    /**
     * Returns whether a shape is locked for user modifications.
     */
    bool isChildLocked(const KoShape *shape) const;

    /**
     * Changes the layout position of a shape that is already contained
     * in this layout.
     */
    void setPosition(const KoShape *shape, uint pos);

    /**
     * Called whenever a property of a shape in this layout has changed.
     *
     * All layout items effected by this change will be re-layouted.
     */
    void childChanged(KoShape *shape, KoShape::ChangeType type);

    /**
     * Does the layouting of shapes that have changed its size or position or
     * that were effected by one of these changes.
     *
     * Only does a relayout if one has been schedules previously through
     * scheduleRelayout().
     *
     * \see scheduleRelayout
     */
    void layout();

    /**
     * Schedules a relayout that is to be done when layout() is called.
     *
     * \see layout
     */
    void scheduleRelayout();

private:
    KoShapeContainer *m_container;
    qreal m_lastWidth;
    bool m_doingLayout;
    bool m_relayoutScheduled;
    KoShape *m_root;
    QList<KoShape*> m_children;
    QList<KoShape*> m_connectors;
    QMap<KoShape*, KoConnectionShape*> m_bonds;
};

#endif // KTREE_LAYOUT_H
