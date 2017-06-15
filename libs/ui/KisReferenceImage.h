/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISREFERENCEIMAGE_H
#define KISREFERENCEIMAGE_H

#include <QScopedPointer>

class QPainter;
class QRectF;
class KisCoordinatesConverter;
class KisCanvas2;

/**
 * @brief The KisReferenceImage class represents a single reference image
 */
class KisReferenceImage
{
public:
    KisReferenceImage();
    ~KisReferenceImage();

    void draw(QPainter &gc, const QRectF &updateRect, const KisCoordinatesConverter *converter, KisCanvas2 *canvas);
private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KISREFERENCEIMAGE_H
