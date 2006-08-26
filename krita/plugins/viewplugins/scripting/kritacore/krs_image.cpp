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

QObject* Image::activePaintLayer()
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

bool Image::convertToColorspace(const QString& colorspacename)
{
    KoColorSpace * dstCS = KisMetaRegistry::instance()->csRegistry()->colorSpace(colorspacename, 0);
    if(!dstCS)
    {
        kWarning() << QString("Colorspace %1 is not available, please check your installation.").arg(colorspacename) << endl;
        return false;
    }
    m_image->convertTo(dstCS);
    return true;
}

void Image::resize(int width, int height, int x, int y)
{
    m_image->resize( width, height, x, y );
}

void Image::scale(double widthfactor, double heightfactor)
{
    m_image->scale( widthfactor, heightfactor, 0, KisFilterStrategyRegistry::instance()->get( "Mitchell") );
}

void Image::rotate(double angle)
{
    m_image->rotate(angle, 0);
}

void Image::shear(double xangle, double yangle)
{
    m_image->shear(xangle, yangle, 0);
}


QObject* Image::createPaintLayer(const QString& name, int opacity)
{
    return createPaintLayer(name, opacity, m_image->colorSpace()->id());
}

QObject* Image::createPaintLayer(const QString& name, int opacity, const QString& colorspacename)
{
    opacity = CLAMP(opacity, 0, 255);
    KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->colorSpace(colorspacename, 0);
    KisPaintLayer* layer = cs ? new KisPaintLayer(m_image.data(), name, opacity, cs)
                              : new KisPaintLayer(m_image.data(), name, opacity);
    layer->setVisible(true);
    m_image->addLayer(KisLayerSP(layer), m_image->rootLayer(), KisLayerSP(0));
    return new PaintLayer(KisPaintLayerSP(layer));
}

#include "krs_image.moc"
