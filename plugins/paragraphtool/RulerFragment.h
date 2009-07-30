/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#ifndef RULERCONTROL_H
#define RULERCONTROL_H

#include <QLineF>
#include <QMatrix>

class Ruler;

class QPainter;
class QPointF;

/* The RulerFragment class represents a Ruler on a specific shape. This class
 * takes care of mapping input and output between the ruler coordinates to
 * the document coordinates. It also takes care of painting this part of the
 * the ruler onto the screen. For each Ruler there may be multiple
 * RulerFragment elements. The ParagraphTool has a ParagraphFragment
 * instance for each shape on which the Rulers need to be painted. Every
 * ParagraphFragment then owns a RulerFragment for each Ruler.
 */
class RulerFragment
{
public:
    RulerFragment() : m_ruler(NULL), m_visible(true) {}
/*
    RulerFragment(Ruler *ruler)
            : m_ruler(ruler),
            m_visible(true) {}
*/
    ~RulerFragment() {}

    bool hitTest(const QPointF &point) const;

    void moveTo(const QPointF &point, bool smoothMovement);

    QLineF labelConnector() const;

    void paint(QPainter &painter) const;

    Ruler *ruler() {
        return m_ruler;
    }

    void setRuler(Ruler *ruler) {
        m_ruler = ruler;
    }
    void setBaseline(const QLineF &baseline);

    bool isVisible() const {
        return m_visible;
    }
    void setVisible(bool visible);

protected:
    void paintArrow(QPainter &painter, const QPointF &tip, const qreal angle, qreal value) const;

    // some convenience methods for rendering the arrow
    static qreal arrowSize();
    static qreal arrowDiagonal();
    static qreal arrowMinimumValue();

private:
    Ruler *m_ruler;
    QMatrix m_matrix;
    qreal m_width;
    bool m_visible;
};

#endif

