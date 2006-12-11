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
    KisPaintDeviceSP paintDevice;
};

KisShapeLayer::KisShapeLayer( KoShapeContainer * parent, KoViewConverter * converter, KisImageSP img, const QString &name, quint8 opacity )
    : KisExternalLayer( img, name, opacity )
{
    KoShapeContainer::setParent( parent );
    setShapeId( KIS_SHAPE_LAYER_ID );

    m_d = new Private();
    m_d->paintDevice = new KisPaintDevice( this, img->colorSpace(), name );
    m_d->converter = converter;
}

KisShapeLayer::~KisShapeLayer()
{
    delete m_d;
}

void KisShapeLayer::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
}

QIcon KisShapeLayer::icon() const
{
    return KIcon("gear");
}

KisPaintDeviceSP KisShapeLayer::prepareProjection(KisPaintDeviceSP projection, const QRect& r)
{
    // For all the contained shapes, render onto a QImage, and
    // composite with the cachec KisPaintDevice
    // If the contained shape is a path shape, use the decorator class
    // Thrain is going to write to render directly onto the
    // KisPaintDevice
    QImage img;
    QPainter p( &img );
    KoShapeContainer::paint( p, *m_d->converter );

    m_d->paintDevice->clear();
    m_d->paintDevice->convertFromQImage( img, "", r.left(), r.top() );

    return m_d->paintDevice;;
}

bool KisShapeLayer::saveToXML(QDomDocument doc, QDomElement elem)
{
    return false;
}

KisLayerSP KisShapeLayer::clone() const
{
    return 0;
}

qint32 KisShapeLayer::x() const
{
    return 0;
}

void KisShapeLayer::setX(qint32)
{
}

qint32 KisShapeLayer::y() const
{
    return 0;
}

void KisShapeLayer::setY(qint32)
{
}

QRect KisShapeLayer::extent() const
{
    return QRect();
}

QRect KisShapeLayer::exactBounds() const
{
    return QRect();
}

bool KisShapeLayer::accept(KisLayerVisitor& visitor)
{
    visitor.visit(this);
}


