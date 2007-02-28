/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "KoShapeConnection.h"
#include "KoShape.h"
#include "KoViewConverter.h"

#include <QPainter>
#include <QPen>
#include <QPointF>

class KoShapeConnection::Private {
public:
    Private(KoShape *from, int gp1, KoShape *to, int gp2)
        : shape1(from),
        shape2(to),
        gluePointIndex1(gp1),
        gluePointIndex2(gp2)
    {
        Q_ASSERT(shape1->connectors().count() > gp1);
        Q_ASSERT(shape2 == 0 || shape2->connectors().count() > gp2);

        zIndex = shape1->zIndex() + 1;
        if(shape2)
            zIndex = qMax(zIndex, shape2->zIndex() + 1);
    }

    KoShape * const shape1;
    KoShape * const shape2;
    int gluePointIndex1;
    int gluePointIndex2;
    QPointF endPoint;
    int zIndex;
};

KoShapeConnection::KoShapeConnection(KoShape *from, int gp1, KoShape *to, int gp2)
    : d(new Private(from, gp1, to, gp2))
{
    d->shape1->addConnection(this);
    d->shape2->addConnection(this);
}

KoShapeConnection::~KoShapeConnection() {
    d->shape1->removeConnection(this);
    d->shape2->removeConnection(this);
    delete d;
}

void KoShapeConnection::paint(QPainter &painter, const KoViewConverter &converter) {
    double x, y;
    converter.zoom(&x, &y);
    QMatrix matrix = d->shape1->transformationMatrix(&converter);
    matrix.scale(x, y);
    QPointF a = matrix.map(d->shape1->connectors()[d->gluePointIndex1]);
    QPointF b;
    if(d->shape2) {
        matrix = d->shape2->transformationMatrix(&converter);
        matrix.scale(x, y);
        b = matrix.map(d->shape2->connectors()[d->gluePointIndex2]);
    }
    else
        b = converter.documentToView(d->endPoint);

    QPen pen(Qt::black);
    painter.setPen(pen);
    painter.drawLine(a, b);
}

KoShape *KoShapeConnection::shape1() const {
    return d->shape1;
}

KoShape *KoShapeConnection::shape2() const {
    return d->shape2;
}

int KoShapeConnection::zIndex() const {
    return d->zIndex;
}

void KoShapeConnection::setZIndex(int index) {
    d->zIndex = index;
}

int KoShapeConnection::gluePointIndex1() const {
    return d->gluePointIndex1;
}

int KoShapeConnection::gluePointIndex2() const {
    return d->gluePointIndex2;
}

QPointF KoShapeConnection::startPoint() const {
    return d->shape1->transformationMatrix(0).map(d->shape1->connectors()[d->gluePointIndex1]);
}

QPointF KoShapeConnection::endPoint() const {
    if(d->shape2)
        return d->shape2->transformationMatrix(0).map(d->shape2->connectors()[d->gluePointIndex2]);
    return d->endPoint;
}

QRectF KoShapeConnection::boundingRect() const {
    QPointF start = startPoint();
    QPointF end = endPoint();
    QRectF br(start.x(), start.y(), end.x() - start.x(), end.y() - start.y());
    return br.normalized();
}

//static
bool KoShapeConnection::compareConnectionZIndex(KoShapeConnection *c1, KoShapeConnection *c2) {
    return c1->zIndex() < c2->zIndex();
}
