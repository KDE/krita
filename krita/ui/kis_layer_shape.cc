/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_layer_shape.h"



#include <kis_types.h>
#include <kis_layer.h>
#include <kis_image.h>

#include <kis_paint_device.h>

#include "kis_mask_shape.h"

class KisLayerShape::Private {
public:
    KisLayerSP layer;
};

KisLayerShape::KisLayerShape( KoShapeContainer * parent, KisLayerSP layer)
    : KoShapeContainer()
{
    m_d = new Private();
    m_d->layer = layer;

    setShapeId( KIS_LAYER_SHAPE_ID );
    setParent( parent );
}

KisLayerShape::~KisLayerShape()
{
    delete m_d;
}

KisLayerSP KisLayerShape::layer()
{
    return m_d->layer;
}

void KisLayerShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}


void KisLayerShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

QSizeF KisLayerShape::size() const
{
    QRect br = m_d->layer->extent();
    KisImageSP image = m_d->layer->image();
    kDebug() << "KisLayerShape::size exactbounds: " << br << ", x res: " << image->xRes() << ", y res: " << image->yRes() << endl;

    return QSizeF( br.width() / image->xRes(), br.height() / image->yRes() );
}

QRectF KisLayerShape::boundingRect() const
{
    QRect br = m_d->layer->extent();
    kDebug() << "KisLayerShape::size exactbounds: " << br << ", x res: " << m_d->layer->image()->xRes() << ", y res: " << m_d->layer->image()->yRes() << endl;

    return QRectF(int(br.left()) / m_d->layer->image()->xRes(), int(br.top()) / m_d->layer->image()->yRes(),
                  int(1 + br.right()) / m_d->layer->image()->xRes(), int(1 + br.bottom()) / m_d->layer->image()->yRes());

}

void KisLayerShape::addChild( KoShape * shape )
{
    if ( shape->shapeId() != KIS_MASK_SHAPE_ID ) {
        kDebug() << "Can only add mask shapes as children to layer shapes!\n";
        return;
    }
}
