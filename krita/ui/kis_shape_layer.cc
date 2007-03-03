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
#include <QPainterPath>
#include <QRect>
#include <QDomElement>
#include <QDomDocument>
#include <QIcon>
#include <QString>
#include <QList>

#include <kicon.h>

#include <KoZoomHandler.h>
#include <KoShapeContainer.h>
#include <KoViewConverter.h>
#include <KoShapeManager.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include "kis_shape_layer_canvas.h"
#include <kis_painter.h>
#include <KoCompositeOp.h>

class KisShapeLayer::Private
{
public:
    KoZoomHandler * converter;
    qint32 x;
    qint32 y;
    KisPaintDeviceSP projection;
    KisShapeLayerCanvas * canvas;
};

KisShapeLayer::KisShapeLayer( KoShapeContainer * parent,
                              KisImageSP img,
                              const QString &name,
                              quint8 opacity )
    : KisExternalLayer( img, name, opacity )
{
    KoShapeContainer::setParent( parent );
    setShapeId( KIS_SHAPE_LAYER_ID );

    m_d = new Private();
    m_d->converter = new KoZoomHandler();
    m_d->x = 0;
    m_d->y = 0;
    m_d->projection = new KisPaintDevice( img->colorSpace() );
    m_d->canvas = new KisShapeLayerCanvas( this, m_d->converter );
    m_d->canvas->setProjection( m_d->projection );
}

KisShapeLayer::~KisShapeLayer()
{
    delete m_d->converter;
    delete m_d->canvas;
    delete m_d;
}

void KisShapeLayer::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED( converter );
    Q_UNUSED( painter );
}

void KisShapeLayer::addChild(KoShape *object)
{
    kDebug(41001) << "KisShapeLayer::addChild {" << endl;
    KoShapeLayer::addChild( object );
    m_d->canvas->shapeManager()->add( object );

    setDirty(m_d->converter->documentToView(object->boundingRect()).toRect());
}

QIcon KisShapeLayer::icon() const
{
    return KIcon("gear");
}

void KisShapeLayer::prepareProjection(const QRect& r)
{
    kDebug(41001) << "KisShapeLayer::prepareProjection()" << r << endl;

    // XXX: Is r in document, widget or pixel coordinates? I hope in
    // document coordinates. Note: see dox for updateCanvas.

    setDirty( r ); // Convert to right coordinates
}

KisPaintDeviceSP KisShapeLayer::projection()
{
    kDebug() << "KisShapeLayer::projection\n";
    return m_d->projection;
}

bool KisShapeLayer::saveToXML(QDomDocument doc, QDomElement elem)
{
#ifdef __GNUC__
    #warning "Implement KisShapeLayer::saveToXML"
#endif
    Q_UNUSED(doc);
    Q_UNUSED(elem);
    return false;
}

KisLayerSP KisShapeLayer::clone() const
{
#ifdef __GNUC__
    #warning "Implement KisShapeLayer::clone()"
#endif
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
    setDirty();
}

qint32 KisShapeLayer::y() const
{
    return m_d->y;
}

void KisShapeLayer::setY(qint32 y)
{
    if ( y == m_d->y ) return;
    m_d->y = y;
    setDirty();
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
