/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2012      Inge Wallin <inge@lysator.liu.se>
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

#ifndef KOSHAPESTROKE_H
#define KOSHAPESTROKE_H

#include "KoFlakeTypes.h"
#include "KoShapeStrokeModel.h"

#include "kritaflake_export.h"

#include <QMetaType>
#include <QColor>


class KoShape;
class QPainter;
class QBrush;
class QPen;
struct KoInsets;

/**
 * A border for shapes that draws a single line around the object.
 */
class KRITAFLAKE_EXPORT KoShapeStroke : public KoShapeStrokeModel
{
public:
    /// Constructor for a thin line in black
    KoShapeStroke();

    /// Copy constructor
    KoShapeStroke(const KoShapeStroke &other);

    /**
     * Constructor for a Stroke
     * @param lineWidth the width, in pt
     * @param color the color we draw the outline in.
     */
    explicit KoShapeStroke(qreal lineWidth, const QColor &color = Qt::black);
    ~KoShapeStroke() override;

    /// Assignment operator
    KoShapeStroke& operator = (const KoShapeStroke &rhs);

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
    QColor color() const;
    /// Sets the color
    void setColor(const QColor &color);

    /// Sets the strokes brush used to fill strokes of this border
    void setLineBrush(const QBrush & brush);
    /// Returns the strokes brush
    QBrush lineBrush() const;

    // pure virtuals from KoShapeStrokeModel implemented here.
    void fillStyle(KoGenStyle &style, KoShapeSavingContext &context) const override;
    void strokeInsets(const KoShape *shape, KoInsets &insets) const override;
    qreal strokeMaxMarkersInset(const KoShape *shape) const override;
    bool hasTransparency() const override;
    void paint(KoShape *shape, QPainter &painter) override;

    QPen resultLinePen() const;

    bool compareFillTo(const KoShapeStrokeModel *other) override;
    bool compareStyleTo(const KoShapeStrokeModel *other) override;
    bool isVisible() const override;

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE( KoShapeStroke )

#endif // KOSHAPESTROKE_H
