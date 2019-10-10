/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisSnapPixelStrategy.h"

#include <QPainterPath>
#include "kis_global.h"
#include "kis_canvas2.h"
#include "KoSnapProxy.h"

KisSnapPixelStrategy::KisSnapPixelStrategy(KoSnapGuide::Strategy type):
    KoSnapStrategy(type)
{
}

KisSnapPixelStrategy::~KisSnapPixelStrategy()
{
}

bool KisSnapPixelStrategy::snap(const QPointF &mousePosition, KoSnapProxy *proxy, qreal maxSnapDistance)
{
    Q_UNUSED(maxSnapDistance);
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2*>(proxy->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas2, false);

    const QPointF imagePos = canvas2->coordinatesConverter()->documentToImage(mousePosition);
    const QPointF alignedDocPoint = canvas2->coordinatesConverter()->imageToDocument(imagePos.toPoint());
    setSnappedPosition(alignedDocPoint);

    return true;
}

QPainterPath KisSnapPixelStrategy::decoration(const KoViewConverter &converter) const
{
    QSizeF unzoomedSize = converter.viewToDocument(QSizeF(5, 5));
    QPainterPath decoration;
    decoration.moveTo(snappedPosition() - QPointF(unzoomedSize.width(), 0));
    decoration.lineTo(snappedPosition() + QPointF(unzoomedSize.width(), 0));
    decoration.moveTo(snappedPosition() - QPointF(0, unzoomedSize.height()));
    decoration.lineTo(snappedPosition() + QPointF(0, unzoomedSize.height()));
    return decoration;
}
