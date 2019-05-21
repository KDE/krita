/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOVECTORPATTERNBACKGROUND_H
#define KOVECTORPATTERNBACKGROUND_H

#include <KoShapeBackground.h>
#include <KoFlakeCoordinateSystem.h>

class KoShape;
class QPointF;
class QRectF;
class QTransform;
class KoVectorPatternBackgroundPrivate;


class KoVectorPatternBackground : public KoShapeBackground
{
public:
    KoVectorPatternBackground();
    ~KoVectorPatternBackground() override;

    bool compareTo(const KoShapeBackground *other) const override;

    void setReferenceCoordinates(KoFlake::CoordinateSystem value);
    KoFlake::CoordinateSystem referenceCoordinates() const;

    /**
     * In ViewBox just use the same mode as for referenceCoordinates
     */
    void setContentCoordinates(KoFlake::CoordinateSystem value);
    KoFlake::CoordinateSystem contentCoordinates() const;

    void setReferenceRect(const QRectF &value);
    QRectF referenceRect() const;

    void setPatternTransform(const QTransform &value);
    QTransform patternTransform() const;

    void setShapes(const QList<KoShape*> value);
    QList<KoShape*> shapes() const;

    void paint(QPainter &painter, const KoViewConverter &converter_Unused, KoShapePaintingContext &context_Unused, const QPainterPath &fillPath) const override;
    bool hasTransparency() const override;
    void fillStyle(KoGenStyle &style, KoShapeSavingContext &context) override;
    bool loadStyle(KoOdfLoadingContext &context, const QSizeF &shapeSize) override;
private:
    SHARED_DECLARE_PRIVATE(KoVectorPatternBackground)
};

#endif // KOVECTORPATTERNBACKGROUND_H
