/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOLINEBORDER_H
#define KOLINEBORDER_H

#include "KoInsets.h"
#include "KoShapeBorderModel.h"

#include "flake_export.h"

class KoShape;
class QPainter;
class QColor;
class KoViewConverter;

/**
 * A border for shapes that draws a single line around the object.
 */
class FLAKE_EXPORT KoLineBorder : public KoShapeBorderModel
{
public:
    /// Constructor for a thin line in black
    KoLineBorder();

    /// Copy constructor
    KoLineBorder(const KoLineBorder &other);

    /**
     * Constructor for a lineBorder
     * @param lineWidth the width, in pt
     * @param color the color we draw the outline in.
     */
    explicit KoLineBorder(qreal lineWidth, const QColor &color = Qt::black);
    virtual ~KoLineBorder();

    /// Assignment operator
    KoLineBorder& operator = (const KoLineBorder &rhs);

    /// Sets the lines cap style
    void setCapStyle(Qt::PenCapStyle style);
    /// Returns the lines cap style
    Qt::PenCapStyle capStyle() const;
    /// Sets the lines join style
    void setJoinStyle(Qt::PenJoinStyle style);
    /// Returns the lines join style
    Qt::PenJoinStyle joinStyle() const;
    /// Sets the line width
    void setLineWidth(qreal lineWidth);
    /// Returns the line width
    qreal lineWidth() const;
    /// Sets the miter limit
    void setMiterLimit(qreal miterLimit);
    /// Returns the miter limit
    qreal miterLimit() const;
    /// Sets the line style
    void setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes);
    /// Returns the line style
    Qt::PenStyle lineStyle() const;
    /// Returns the line dashes
    QVector<qreal> lineDashes() const;
    /// Sets the dash offset
    void setDashOffset(qreal dashOffset);
    /// Returns the dash offset
    qreal dashOffset() const;

    /// Returns the color
    const QColor & color() const;
    /// Sets the color
    void setColor(const QColor & color);

    /// Sets the strokes brush used to fill strokes of this border
    void setLineBrush(const QBrush & brush);
    /// Returns the strokes brush
    QBrush lineBrush() const;

    virtual void fillStyle(KoGenStyle &style, KoShapeSavingContext &context) const;
    virtual void borderInsets(const KoShape *shape, KoInsets &insets) const;
    virtual bool hasTransparency() const;

    virtual void paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter);
    virtual void paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter, const QColor &color);

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE( KoLineBorder )

#endif
