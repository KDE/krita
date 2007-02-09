/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOEMPTYBORDER_H
#define KOEMPTYBORDER_H

#include "KoInsets.h"
#include "KoShapeBorderModel.h"

#include <flake_export.h>

class KoShape;
class QPainter;
class QColor;
class QPen;
class KoViewConverter;

/**
 * A border for shapes that draws a single line around the object.
 */
class FLAKE_EXPORT KoLineBorder : public KoShapeBorderModel {
public:
    /// Constructor for a thin line in black
    KoLineBorder();
    /**
     * Constructor for a lineBorder
     * @param lineWidth the width, in pt
     * @param color the color we draw the outline in.
     */
    explicit KoLineBorder(double lineWidth, QColor color = Qt::black);
    virtual ~KoLineBorder() {};

    /// Sets the lines cap style
    void setCapStyle( Qt::PenCapStyle style );
    /// Returns the lines cap style
    Qt::PenCapStyle capStyle() const;
    /// Sets the lines join style
    void setJoinStyle( Qt::PenJoinStyle style );
    /// Returns the lines join style
    Qt::PenJoinStyle joinStyle() const;
    /// Sets the line width
    void setLineWidth( double lineWidth );
    /// Returns the line width
    double lineWidth() const;
    /// Sets the miter limit
    void setMiterLimit( double miterLimit );
    /// Returns the miter limit
    double miterLimit() const;

    /// Returns the color
    const QColor & color() const;
    /// Stes the color
    void setColor( const QColor & color );

    virtual KoInsets* borderInsets(const KoShape *shape, KoInsets &insets);
    virtual bool hasTransparency();
    virtual void paintBorder(KoShape *shape, QPainter &painter, const KoViewConverter &converter);

private:
    QColor m_color;
    QPen m_pen;
};

#endif
