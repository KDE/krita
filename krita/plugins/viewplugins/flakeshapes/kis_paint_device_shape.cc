/*
 * kis_paintdevice_shape.h -- Part of Krita
 *
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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


//   #include <QString>
//   #include <QWidget>
//   #include <QPixmap>
//   #include <QList>
//   #include <QPainter>
//   #include <QSize>

#include <klocale.h>
#include <kgenericfactory.h>

//   #include <KoViewConverter.h>
//   #include <KoShape.h>
//   #include <KoShapeFactory.h>

#include "kis_paint_device.h"
//   #include "kis_fill_painter.h"
//   #include "kis_colorspace.h"
//   #include "kis_colorspace_factory_registry.h"
#include "kis_paint_device_shape.h"

KisPaintDeviceShape::KisPaintDeviceShape()
    : KoShape()
{
/*
    KisColorSpace * cs = KisColorspaceFactoryRegistry::instance()->rgb8();
    m_device = new KisPaintDevice(cs, "Krita PaintDevice Shape");

    KisFillPainter painter;
    painter.begin(m_device);
    painter.fillRect(0, 0, 256, 256, KisColor(Qt::white, cs), OPACITY_OPAQUE);
    painter.end();

    resize( QSize( 256, 256 ) );

*/
}

void KisPaintDeviceShape::paint(QPainter &painter, KoViewConverter &converter)
{
    // KoViewConvert: has zoom() method to get zoomlevel

    applyConversion( painter,  converter );
    // XXX: We pass 0 for the display profile for now: this should be
    //      the real monitor profile.
    painter.drawImage( QRect( QPoint( 0, 0 ), size().toSize() ),
                       m_device->convertToQImage( 0 ) );
}


KisPaintDeviceShapeFactory::KisPaintDeviceShapeFactory(QObject *parent,  const QStringList &)
: KoShapeFactory(parent, "KisPaintDeviceShape", i18n( "Krita Paintdevice" ) )
{
    setToolTip( i18n( "A surface to paint on with pixels" ) );
    // setPixmap(); // XXX: load icon using KIconLoader
    // XXX: Set default templates

}


KoShape * KisPaintDeviceShapeFactory::createDefaultShape()
{
    return new KisPaintDeviceShape();
}

KoShape * KisPaintDeviceShapeFactory::createShape(const KoProperties * params) const
{
    return new KisPaintDeviceShape();
}

QWidget * KisPaintDeviceShapeFactory::optionWidget()
{
    return 0;
}
