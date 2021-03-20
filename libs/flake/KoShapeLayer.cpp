/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2007 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoShapeLayer.h"

#include <QRectF>

#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>

KoShapeLayer::KoShapeLayer()
        : KoShapeContainer(new SimpleShapeContainerModel())
{
    setSelectable(false);
}

KoShapeLayer::KoShapeLayer(KoShapeContainerModel *model)
        : KoShapeContainer(model)
{
    setSelectable(false);
}

bool KoShapeLayer::hitTest(const QPointF &position) const
{
    Q_UNUSED(position);
    return false;
}

QRectF KoShapeLayer::boundingRect() const
{
    return KoShape::boundingRect(shapes());
}

void KoShapeLayer::paintComponent(QPainter &, KoShapePaintingContext &) const
{
}
