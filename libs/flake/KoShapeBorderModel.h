/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPEBORDERMODEL_H
#define KOSHAPEBORDERMODEL_H

#include "KoInsets.h"

#include <QPainter>

#include <koffice_export.h>

class KoShape;
class KoViewConverter;

/**
 * A model for borders around KoShapes.
 * Classes that implement this model will be allowed to draw a border on the outline
 * of a shape.
 * Note that since the important members take a KoShape as argument it is possible,
 * and preferred behavior, to have one instance of a border that is reused on several
 * objects.
 */
class FLAKE_EXPORT KoShapeBorderModel {
public:
    KoShapeBorderModel() {};
    virtual ~KoShapeBorderModel() {};

    /**
     * Return a new borderInsets object filled with the size around the shape that this
     * border takes.
     * @param shape the shape the insets will be calculated for
     * Note that the KoInsets is a new object that you are responsible to delete afterwards.
     */
    KoInsets* borderInsets(const KoShape *shape) {
        KoInsets *insets = new KoInsets(0, 0, 0, 0);
        borderInsets(shape, *insets);
        return insets;
    };
    /**
     * Return a borderInsets object filled with the size around the shape that this border takes.
     * @param shape the shape the insets will be calculated for
     * @param insets the insets object that will be filled and returned.
     */
    virtual KoInsets* borderInsets(const KoShape *shape, KoInsets &insets) = 0;
    /**
     * Returns true if there is some transparency, false if the border is fully opaque.
     * @return if the border is transparent.
     */
    virtual bool hasTransparency() = 0;
    /**
     * Paint the border.
     * This method should paint the border around shape.
     * @param shape the shape to paint around
     * @param painter the painter to paint to, the painter will have the topleft of the
     *       shape as its start coordinate.
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paintBorder(KoShape *shape, QPainter &painter, const KoViewConverter &converter) = 0;
};

#endif
