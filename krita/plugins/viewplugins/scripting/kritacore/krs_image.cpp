/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_image.h"

#include <klocale.h>
#include <kdebug.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>
#include <kis_filter_strategy.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_meta_registry.h>

#include "krs_paint_layer.h"

using namespace Kross::KritaCore;

Image::Image(KisImageSP image, KisDoc* doc)
    : QObject()
    , m_image(image)
    , m_doc(doc)
{
    setObjectName("KritaImage");
}

Image::~Image()
{
}

QObject* Image::getActivePaintLayer()
{
    KisPaintLayer* activePaintLayer = dynamic_cast< KisPaintLayer* >(m_image->activeLayer().data());
    if(activePaintLayer)
        return new PaintLayer(KisPaintLayerSP(activePaintLayer), m_doc);
    kWarning() << "The active layer is not paintable." << endl;
    return 0;
}

int Image::width() const
{
    return m_image->width();
}

int Image::height() const
{
    return m_image->height();
}

QString Image::colorSpaceId() const
{
    return m_image->colorSpace()->id();
}

#if 0
Kross::Api::Object::Ptr Image::convertToColorspace(Kross::Api::List::Ptr args)
{
    KoColorSpace * dstCS = KisMetaRegistry::instance()->csRegistry()->colorSpace(Kross::Api::Variant::toString(args->item(0)), 0);
    if(!dstCS)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Colorspace %1 is not available, please check your installation.",Kross::Api::Variant::toString(args->item(0))) ) );
        return Kross::Api::Object::Ptr(0);
    }
    m_image->convertTo(dstCS);
    return Kross::Api::Object::Ptr(0);
}

Kross::Api::Object::Ptr Image::createPaintLayer(Kross::Api::List::Ptr args)
{
    QString name = Kross::Api::Variant::toString(args->item(0));
    int opacity = Kross::Api::Variant::toInt(args->item(1));
    opacity = CLAMP(opacity, 0, 255);
    QString csname;
    if(args->count() > 2)
    {
        csname = Kross::Api::Variant::toString(args->item(2));
    } else {
        csname = m_image->colorSpace()->id();
    }
    KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->colorSpace(csname, 0);
    KisPaintLayer* layer;
    if(cs)
    {
        layer = new KisPaintLayer(m_image.data(), name, opacity, cs);
    } else {
        layer = new KisPaintLayer(m_image.data(), name, opacity);
    }
    layer->setVisible(true);

    m_image->addLayer(KisLayerSP(layer), m_image->rootLayer(), KisLayerSP(0));
    return Kross::Api::Object::Ptr(new PaintLayer(KisPaintLayerSP(layer)));

}

Kross::Api::Object::Ptr Image::scale(Kross::Api::List::Ptr args)
{
    double cw = Kross::Api::Variant::toDouble(args->item(0));
    double ch = Kross::Api::Variant::toDouble(args->item(1));
    m_image->scale( cw, ch, 0, KisFilterStrategyRegistry::instance()->get( "Mitchell") );
    return Kross::Api::Object::Ptr(0);
}
Kross::Api::Object::Ptr Image::resize(Kross::Api::List::Ptr args)
{
    int nw = Kross::Api::Variant::toInt(args->item(0));
    int nh = Kross::Api::Variant::toInt(args->item(1));
    int x = 0;
    int y = 0;
    if(args->count() > 2)
    {
        x = Kross::Api::Variant::toInt(args->item(2));
        y = Kross::Api::Variant::toInt(args->item(3));
    }
    m_image->resize( nw, nh, x, y );
    return Kross::Api::Object::Ptr(0);
}
#endif

#include "krs_image.moc"
