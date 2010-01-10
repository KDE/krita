/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoCreateShapesTool.h"

#include <QMouseEvent>
#include <QPainter>

#include "KoPointerEvent.h"
#include "KoInteractionStrategy.h"
#include "KoCreateShapeStrategy.h"


class KoCreateShapesTool::Private
{
public:
    Private() : newShapeProperties(0) {}

    QString shapeId;
    KoProperties *newShapeProperties;
};

KoCreateShapesTool::KoCreateShapesTool(KoCanvasBase *canvas)
        : KoInteractionTool(canvas),
        d(new Private())
{
}

KoCreateShapesTool::~KoCreateShapesTool()
{
    delete d;
}

void KoCreateShapesTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (currentStrategy())
        currentStrategy()->paint(painter, converter);
}

void KoCreateShapesTool::mouseReleaseEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseReleaseEvent(event);
    emit KoTool::done();
}

void KoCreateShapesTool::activate(bool)
{
    useCursor(Qt::ArrowCursor);
}

void KoCreateShapesTool::setShapeId(const QString &id)
{
    d->shapeId = id;
}

QString KoCreateShapesTool::shapeId() const
{
    return d->shapeId;
}

void KoCreateShapesTool::setShapeProperties(KoProperties *properties)
{
    d->newShapeProperties = properties;
}

KoProperties const * KoCreateShapesTool::shapeProperties()
{
    return d->newShapeProperties;
}

KoInteractionStrategy *KoCreateShapesTool::createStrategy(KoPointerEvent *event)
{
    return new KoCreateShapeStrategy(this, event->point);
}

