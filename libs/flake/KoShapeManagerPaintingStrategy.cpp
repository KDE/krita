/* This file is part of the KDE project

   Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoShapeManagerPaintingStrategy.h"

#include "KoShape.h"
#include "KoShapeManager.h"
#include <QPainter>

class KoShapeManagerPaintingStrategy::Private
{
public:
    Private(KoShapeManager * manager)
    : shapeManager(manager)
    {}

    KoShapeManager * shapeManager;
};

KoShapeManagerPaintingStrategy::KoShapeManagerPaintingStrategy(KoShapeManager * shapeManager)
: d(new KoShapeManagerPaintingStrategy::Private(shapeManager))
{
}

KoShapeManagerPaintingStrategy::~KoShapeManagerPaintingStrategy()
{
    delete d;
}

void KoShapeManagerPaintingStrategy::paint(KoShape * shape, QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    painter.save();
    painter.setTransform(shape->absoluteTransformation(&converter) * painter.transform());

    if (d->shapeManager) {
        d->shapeManager->paintShape(shape, painter, converter, forPrint);
    }

    painter.restore();  // for the matrix
}

void KoShapeManagerPaintingStrategy::adapt(KoShape * shape, QRectF & rect)
{
    Q_UNUSED(shape);
    Q_UNUSED(rect);
}

void KoShapeManagerPaintingStrategy::setShapeManager(KoShapeManager * shapeManager)
{
    d->shapeManager = shapeManager;
}

KoShapeManager * KoShapeManagerPaintingStrategy::shapeManager()
{
    return d->shapeManager;
}
