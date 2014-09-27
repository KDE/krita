/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#ifndef KOSHAPESTROKEMODEL_H
#define KOSHAPESTROKEMODEL_H

#include "flake_export.h"

class KoShape;
class KoGenStyle;
class KoShapeSavingContext;
class KoViewConverter;
struct KoInsets;

class QColor;
class QPainter;

/**
 * A model for strokes of KoShapes.
 * Classes that implement this model will be allowed to draw the stroke of the outline
 * of a shape.
 * Note that since the important members take a KoShape as argument it is possible,
 * and preferred behavior, to have one instance of a stroke that is reused on several
 * objects.
 */
class FLAKE_EXPORT KoShapeStrokeModel
{
public:
    KoShapeStrokeModel();
    virtual ~KoShapeStrokeModel();

    /**
     * @brief Fill the style object (aka save)
     *
     * @param style object
     * @param context used for saving
     */
    virtual void fillStyle(KoGenStyle &style, KoShapeSavingContext &context) const = 0;

    /**
     * Return a strokeInsets object filled with the size inside the shape that this stroke takes.
     * @param shape the shape the insets will be calculated for
     * @param insets the insets object that will be filled and returned.
     */
    virtual void strokeInsets(const KoShape *shape, KoInsets &insets) const = 0;
    /**
     * Returns true if there is some transparency, false if the stroke is fully opaque.
     * @return if the stroke is transparent.
     */
    virtual bool hasTransparency() const = 0;
    /**
     * Paint the stroke.
     * This method should paint the stroke around shape.
     * @param shape the shape to paint around
     * @param painter the painter to paint to, the painter will have the topleft of the
     *       shape as its start coordinate.
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter) = 0;

    /**
     * Paint the stroke in the given color
     *
     * This method should paint the stroke around the shape in the given color.
     *
     * @param shape the shape to paint around
     * @param painter the painter to paint to, the painter will have the topleft of the
     *       shape as its start coordinate.
     * @param converter to convert between internal and view coordinates.
     * @param color to use to paint the stroke.
     */
    virtual void paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter, const QColor &color) = 0;

    /**
     * Increments the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool ref();
    /**
     * Decrements the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool deref();
    /// Return the usage count
    int useCount() const;

private:
    class Private;
    Private * const d;
};

#endif
