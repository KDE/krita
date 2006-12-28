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
#include "kis_shape_layer.h"

#include <QPainter>
#include <QRect>
#include <QDomElement>
#include <QDomDocument>
#include <QIcon>
#include <QString>

#include <kicon.h>

#include <KoShapeContainer.h>
#include <KoViewConverter.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>

class KisShapeLayer::Private
{
public:
    KoViewConverter * converter;
    qint32 x;
    qint32 y;
};

KisShapeLayer::KisShapeLayer( KoShapeContainer * parent, KoViewConverter * converter, KisImageSP img, const QString &name, quint8 opacity )
    : KisExternalLayer( img, name, opacity )
{
    KoShapeContainer::setParent( parent );
    setShapeId( KIS_SHAPE_LAYER_ID );

    m_d = new Private();
    m_d->converter = converter;
    m_d->x = 0;
    m_d->y = 0;
}

KisShapeLayer::~KisShapeLayer()
{
    delete m_d;
}

void KisShapeLayer::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    kDebug() << "KisShapeLayer::paintComponent " << boundingRect() << endl;
    painter.fillRect( boundingRect(), QColor(200, 100, 100, 100) );
    kDebug() << "KisShapeLayer::paintComponent done" << endl;
}

void KisShapeLayer::addChild(KoShape *object)
{
    kDebug() << "KisShapeLayer::addChild " << object->shapeId() << ", " << object->boundingRect() << endl;
    KoShapeLayer::addChild( object );
    setDirty( object->boundingRect().toRect(), true ); // XXX: convert to pixels
}

QIcon KisShapeLayer::icon() const
{
    return KIcon("gear");
}

KisPaintDeviceSP KisShapeLayer::prepareProjection(KisPaintDeviceSP projection, const QRect& r)
{
    kDebug() << "KisShapeLayer::prepareProjection " << r << endl;
    QPainter p( projection.data() );
    KoShapeLayer::paint( p, *m_d->converter );

    return projection;
}

bool KisShapeLayer::saveToXML(QDomDocument doc, QDomElement elem)
{
    #warning "Implement KisShapeLayer::saveToXML"
    Q_UNUSED(doc);
    Q_UNUSED(elem);
    return false;
}

KisLayerSP KisShapeLayer::clone() const
{
    #warning "Implement KisShapeLayer::clone()"
    return 0;
}

qint32 KisShapeLayer::x() const
{
    return m_d->x;
}

void KisShapeLayer::setX(qint32 x)
{
    if ( x == m_d->x ) return;
    m_d->x = x;
    setDirty( true );
}

qint32 KisShapeLayer::y() const
{
    return m_d->y;
}

void KisShapeLayer::setY(qint32 y)
{
    if ( y == m_d->y ) return;
    m_d->y = y;
    setDirty( true );
}

QRect KisShapeLayer::extent() const
{
    QRect rc = boundingRect().toRect();
    return QRectF( rc.x() * image()->xRes(), rc.y() * image()->yRes(), rc.width() * image()->xRes(), rc.height() * image()->yRes() ).toRect();
}

QRect KisShapeLayer::exactBounds() const
{
    QRect rc = boundingRect().toRect();
    return QRectF( rc.x() * image()->xRes(), rc.y() * image()->yRes(), rc.width() * image()->xRes(), rc.height() * image()->yRes() ).toRect();
}

bool KisShapeLayer::accept(KisLayerVisitor& visitor)
{
    return visitor.visit(this);
}
