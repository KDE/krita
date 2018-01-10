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

#include <KoShape.h>

class QImage;
class QPointF;
class QPainter;
class QRectF;
class KisCoordinatesConverter;
class KisCanvas2;

/**
 * @brief The KisReferenceImage class represents a single reference image
 */
class KisReferenceImage : public KoShape
{
public:
    KisReferenceImage();
    ~KisReferenceImage();

    void setImage(QImage image);
    void setPosition(QPointF pos);
    void setGrayscale(bool grayscale);

    void paint(QPainter &gc, const KoViewConverter &converter, KoShapePaintingContext &paintcontext) override;

    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) override { return false; }
    void saveOdf(KoShapeSavingContext &context) const override {}

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KISREFERENCEIMAGE_H
