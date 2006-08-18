/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoPathShapeFactory.h"
#include "KoPathShape.h"
#include "KoLineBorder.h"

#include <klocale.h>

KoPathShapeFactory::KoPathShapeFactory(QObject *parent, const QStringList&)
: KoShapeFactory(parent, KoPathShapeId, i18n("A simple path shape"))
{
    setToolTip("A simple path shape");
}

KoShape * KoPathShapeFactory::createDefaultShape() {
    KoPathShape* path = new KoPathShape();
    path->moveTo( QPointF( 0, 10 ) );
    path->curveTo( QPointF( 0, 20 ), QPointF( 5, 20 ), QPointF( 5, 10 ) );
    path->curveTo( QPointF( 5, 0 ), QPointF( 10, 0 ), QPointF( 10, 10 ) );
    path->normalize();
    path->setBorder( new KoLineBorder( 1.0 ) );
    path->setShapeId(shapeId());
    return path;
}

KoShape * KoPathShapeFactory::createShape(const KoProperties * params) const {
    Q_UNUSED(params);
    return new KoPathShape();
}

