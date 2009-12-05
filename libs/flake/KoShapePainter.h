/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPEPAINTER_H
#define KOSHAPEPAINTER_H

#include "flake_export.h"

#include <QtCore/QList>
#include <QtCore/QRectF>

class KoShape;
class KoViewConverter;
class KoShapeManagerPaintingStrategy;
class QPainter;
class QImage;

/**
 * A utility class to paint a subset of shapes onto a QPainter.
 * Notice that using setShapes repeatedly is very expensive, as it populates
 * the shapeManager and all its caching every time.  If at all possible use
 * a shapeManager directly and avoid loosing the cache between usages.
 */
class FLAKE_EXPORT KoShapePainter
{
public:
    KoShapePainter(KoShapeManagerPaintingStrategy * strategy = 0);
    ~KoShapePainter();

    /**
     * Sets the shapes to be painted.
     * @param shape the shapes to paint
     */
    void setShapes(const QList<KoShape*> &shapes);

    /**
     * Paints the shapes on the given painter and using the zoom handler.
     * @param painter the painter to paint on
     * @param converted the view converter defining the zoom to use
     */
    void paintShapes(QPainter &painter, KoViewConverter &converter);

    /**
     * Paints the shapes on the given painter.
     * The given document rectangle is painted to fit into the given painter rectangle.
     *
     * @param painter the painter to paint on
     * @param painterRect the destination rectangle on the painter
     * @param documentRect the document region to paint
     */
    void paintShapes(QPainter &painter, const QRect &painterRect, const QRectF &documentRect);

    /**
     * Paints shapes to the given image, so that all shapes fit onto it.
     * @param image the image to paint into
     * @return false if image is empty, else true
     */
    void paintShapes(QImage &image);

    /// Returns the bounding rect of the shapes to paint
    QRectF contentRect();

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPEPAINTER_H
