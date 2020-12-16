/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOVECTORPATTERNBACKGROUND_H
#define KOVECTORPATTERNBACKGROUND_H

#include <KoShapeBackground.h>
#include <KoFlakeCoordinateSystem.h>
#include <QSharedDataPointer>

class KoShape;
class QPointF;
class QRectF;
class QTransform;


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

    void paint(QPainter &painter, KoShapePaintingContext &context_Unused, const QPainterPath &fillPath) const override;
    bool hasTransparency() const override;
private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOVECTORPATTERNBACKGROUND_H
