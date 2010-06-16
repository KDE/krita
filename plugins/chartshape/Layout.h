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

#ifndef KCHART_LAYOUT_H
#define KCHART_LAYOUT_H

// Qt
#include <QList>
#include <QMap>

// KOffice
#include <KoShapeContainerModel.h>

// KChart
#include "kchart_global.h"


class QSizeF;


namespace KChart {

/**
 * A generic chart-style layout with 10 possible positions:
 *
 *  ----------------------
 * | A |       D      | F |
 * |---|------------------|
 * |   |              |   |
 * | B |       I      | G |      (J)
 * |   |              |   |
 * |---|------------------|
 * | C |       E      | H |
 *  ----------------------
 *
 * A - TopStartPosition
 * B - StartPosition
 * C - BottomStartPosition
 * D - TopPosition
 * E - BottomPosition
 * F - TopEndPosition
 * G - EndPosition
 * H - BottomEndPosition
 * I - CenterPosition
 * J - FloatingPosition
 *
 * Layout elements with the same position that are in one of the positions B, D, G or E
 * will be placed more towards the center based on a "weight".
 */
class Layout : public KoShapeContainerModel
{
public:
    Layout();
    ~Layout();

    /**
     * Adds a floating shape to the layout.
     */
    void add( KoShape *shape );

    /**
     * Adds a shape to the layout.
     *
     * @param pos    position in the layout
     * @param weight priority of this shape in regard to its placement when
     *               other shapes are in the same Position.
     *               A shape with a higher weight will be placed more towards
     *               the center (i.e., it "sinks" due to its higher weight)
     */
    void add( KoShape *shape, Position pos, int weight = 0 );

    /**
     * Removes a shape from the layout.
     */
    void remove( KoShape *shape );

    /**
     * Turns clipping of a shape on or off.
     */
    void setClipped( const KoShape *shape, bool clipping );

    /**
     * @see setClipping
     */
    bool isClipped( const KoShape *shape ) const;

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
     * Called whenever a property of the container (i.e. the ChartShape) is changed.
     */
    void containerChanged( KoShapeContainer *container, KoShape::ChangeType type );

    /**
     * Returns whether a shape is locked for user modifications.
     */
    bool isChildLocked( const KoShape *shape ) const;

    /**
     * Changes the layout position of a shape that is already contained
     * in this layout.
     */
    void setPosition( const KoShape *shape, Position pos, int weight = 0 );

    /**
     * Called whenever a property of a shape in this layout has changed.
     *
     * All layout items effected by this change will be re-layouted.
     */
    void childChanged( KoShape *shape, KoShape::ChangeType type );

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
    /**
     * Lays out all items in TopPosition, and returns the y value of
     * the bottom-most item's bottom.
     */
    qreal layoutTop( const QMap<int, KoShape*>& shapes );

    /**
     * Lays out all items in BottomPosition, and returns the y value of
     * the top-most item's top.
     */
    qreal layoutBottom( const QMap<int, KoShape*>& shapes );

    /**
     * Lays out all items in StartPosition, and returns the x value of
     * the right-most item's right.
     */
    qreal layoutStart( const QMap<int, KoShape*>& shapes );

    /**
     * Lays out all items in EndPosition, and returns the x value of
     * the left-most item's left.
     */
    qreal layoutEnd( const QMap<int, KoShape*>& shapes );

    void layoutTopStart( KoShape *shape );
    void layoutBottomStart( KoShape *shape );
    void layoutTopEnd( KoShape *shape );
    void layoutBottomEnd( KoShape *shape );

    class LayoutData;
    bool m_doingLayout;
    bool m_relayoutScheduled;
    QSizeF m_containerSize;
    QMap<KoShape*, LayoutData*> m_layoutItems;
};

} // namespace KChart

#endif // KCHART_CHARTLAYOUT_H
