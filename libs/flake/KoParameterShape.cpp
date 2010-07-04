/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoParameterShape.h"
#include "KoParameterShape_p.h"

#include <QPainter>
#include <KDebug>

KoParameterShape::KoParameterShape()
    : KoPathShape(*(new KoParameterShapePrivate(this)))
{
}

KoParameterShape::KoParameterShape(KoParameterShapePrivate &dd)
    : KoPathShape(dd)
{
}

KoParameterShape::~KoParameterShape()
{
}

void KoParameterShape::moveHandle(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers)
{
    Q_D(KoParameterShape);
    if (handleId >= d->handles.size()) {
        kWarning(30006) << "handleId out of bounds";
        return;
    }

    update();
    // function to do special stuff
    moveHandleAction(handleId, documentToShape(point), modifiers);

    updatePath(size());
    update();
    d->shapeChanged(ParameterChanged);
}


int KoParameterShape::handleIdAt(const QRectF & rect) const
{
    Q_D(const KoParameterShape);
    int handle = -1;

    for (int i = 0; i < d->handles.size(); ++i) {
        if (rect.contains(d->handles.at(i))) {
            handle = i;
            break;
        }
    }
    return handle;
}

QPointF KoParameterShape::handlePosition(int handleId)
{
    Q_D(KoParameterShape);
    return d->handles.value(handleId);
}

void KoParameterShape::paintHandles(QPainter & painter, const KoViewConverter & converter, int handleRadius)
{
    Q_D(KoParameterShape);
    applyConversion(painter, converter);

    QTransform worldMatrix = painter.worldTransform();
    painter.setTransform(QTransform());

    QTransform matrix;
    matrix.rotate(45.0);
    QPolygonF poly(d->handleRect(QPointF(0, 0), handleRadius));
    poly = matrix.map(poly);

    QList<QPointF>::const_iterator it(d->handles.constBegin());
    for (; it != d->handles.constEnd(); ++it) {
        QPointF moveVector = worldMatrix.map(*it);
        poly.translate(moveVector.x(), moveVector.y());
        painter.drawPolygon(poly);
        poly.translate(-moveVector.x(), -moveVector.y());
    }
}

void KoParameterShape::paintHandle(QPainter & painter, const KoViewConverter & converter, int handleId, int handleRadius)
{
    Q_D(KoParameterShape);
    applyConversion(painter, converter);

    QTransform worldMatrix = painter.worldTransform();
    painter.setTransform(QTransform());

    QTransform matrix;
    matrix.rotate(45.0);
    QPolygonF poly(d->handleRect(QPointF(0, 0), handleRadius));
    poly = matrix.map(poly);
    poly.translate(worldMatrix.map(d->handles[handleId]));
    painter.drawPolygon(poly);
}

void KoParameterShape::setSize(const QSizeF &newSize)
{
    Q_D(KoParameterShape);
    QTransform matrix(resizeMatrix(newSize));

    for (int i = 0; i < d->handles.size(); ++i) {
        d->handles[i] = matrix.map(d->handles[i]);
    }

    KoPathShape::setSize(newSize);
}

QPointF KoParameterShape::normalize()
{
    Q_D(KoParameterShape);
    QPointF offset(KoPathShape::normalize());
    QTransform matrix;
    matrix.translate(-offset.x(), -offset.y());

    for (int i = 0; i < d->handles.size(); ++i) {
        d->handles[i] = matrix.map(d->handles[i]);
    }

    return offset;
}

bool KoParameterShape::isParametricShape() const
{
    Q_D(const KoParameterShape);
    return d->parametric;
}

void KoParameterShape::setParametricShape(bool parametric)
{
    Q_D(KoParameterShape);
    d->parametric = parametric;
    update();
}

QList<QPointF> KoParameterShape::handles() const
{
    Q_D(const KoParameterShape);
    return d->handles;
}

void KoParameterShape::setHandles(const QList<QPointF> &handles)
{
    Q_D(KoParameterShape);
    d->handles = handles;
}

int KoParameterShape::handleCount() const
{
    Q_D(const KoParameterShape);
    return d->handles.count();
}
