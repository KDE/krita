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
    Private(KoShape *from, KoShape *to) : shape1(from), shape2(to) {}

    KoShape * const shape1;
    KoShape * const shape2;
    QPointF point1, point2;

    /*
      Properties like ConnectionType
    */
};

KoShapeConnection::KoShapeConnection(KoShape *from, KoShape *to)
    : d(new Private(from, to))
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
    QPointF a = converter.documentToView(d->shape1->absolutePosition());
    QPointF b = converter.documentToView(d->shape2->absolutePosition());

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

