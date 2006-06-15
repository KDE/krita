/*
 * kis_image_flake.h -- Part of Krita
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
#include <KoShapeRegistry.h>

//   #include "kis_fill_painter.h"

//#include "kis_colorspace.h"
//   #include "kis_colorspace_factory_registry.h"
#include "kis_image_shape.h"
#include "kis_paint_device_shape.h"

KisImageShape::KisImageShape()
    : KoShape()
{
/*
    KisColorSpace * cs = KisColorspaceFactoryRegistry::instance()->getRGB8();
    m_image = new KisImage(cs, "Krita Image Shape");

    if ( m_image && m_image->rootLayer() && m_image->rootLayer()->activeDevice() ) {
        KisFillPainter painter;
        painter.begin(m_image->rootLayer()->activeDevice());
        painter.fillRect(0, 0, 256, 256, KisColor(Qt::white, cs), OPACITY_OPAQUE);
        painter.end();

        resize( QSize( 256, 256 ) );
    }
*/

}

void KisImageShape::paint(QPainter &painter, KoViewConverter &converter)
{
    applyConversion( painter,  converter );
    // XXX: We pass 0 for the display profile for now: this should be
    //      the real monitor profile.
    // XXX: Here I _need_ to know the zoom level: we need to follow
    //      different optimization paths for different zoom levels
    // XXX: Keep sizes in mind.
    //m_image->renderToPainter( 0, 0,  256, 256, painter );
}




K_EXPORT_COMPONENT_FACTORY(kritaflakeshapes,
     KGenericFactory<KisImageShapeFactory>( "KritaImageShape" ) )
KisImageShapeFactory::KisImageShapeFactory(QObject *parent,  const QStringList & sl)
: KoShapeFactory(parent, "KisImageShape", i18n("Krita Paintimage"))
{
    setToolTip( i18n( "A surface to paint on with pixels" ) );
    // setIcon(); // XXX: use KIconLoader...
    // XXX: Set default templates

    KoShapeRegistry::instance()->add( new KisPaintDeviceShapeFactory(parent, sl) );
}


KoShape * KisImageShapeFactory::createDefaultShape()
{
    return new KisImageShape();
}

KoShape * KisImageShapeFactory::createShape(const KoProperties * params) const
{
    return new KisImageShape();
}

QWidget * KisImageShapeFactory::optionWidget()
{
    return 0;
}
