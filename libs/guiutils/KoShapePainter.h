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

#include "koguiutils_export.h"

#include <QtCore/QList>
#include <QtCore/QRectF>

class KoShape;
class KoViewConverter;
class QPainter;
class QImage;

/**
 * A utility class to paint a subset of shapes onto a QPainter.
 */
class KOGUIUTILS_EXPORT KoShapePainter
{
public:
    KoShapePainter();
    ~KoShapePainter();
    void setShapes( const QList<KoShape*> &shapes );
    /// paints the shapes on the given painter and using the zoom handler
    void paintShapes( QPainter & painter, KoViewConverter & converter );
    /// paints shapes to the given image, so that all shapes fit onto it
    bool paintShapes( QImage & image );
    /// returns the bounding rect of the shapes to paint
    QRectF contentRect();

private:
    class Private;
    Private const * d;
};

#endif // KOSHAPEPAINTER_H
