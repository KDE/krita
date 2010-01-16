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

#include "krs_paint_layer.h"
#include "krs_module.h"

#include <klocale.h>
#include <kis_debug.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_filter_strategy.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>


using namespace Scripting;

Image::Image(Module* module, KisImageWSP image, KisDoc2* doc)
        : QObject(module)
        , m_image(image)
        , m_doc(doc)
{
    setObjectName("KritaImage");
}

Image::~Image()
{
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

bool Image::convertToColorSpace(const QString& model, const QString& depth)
{
    const KoColorSpace * dstCS = KoColorSpaceRegistry::instance()->colorSpace(model, depth, 0);
    if (!dstCS) {
        warnScript << QString("ColorSpace %1 %2 is not available, please check your installation.").arg(model).arg(depth) << endl;
        return false;
    }
    m_image->convertTo(dstCS);
    return true;
}

void Image::resize(int width, int height, int x, int y)
{
    m_image->resize(width, height, x, y);
}

void Image::scale(double widthfactor, double heightfactor)
{
    m_image->scale(widthfactor, heightfactor, 0, KisFilterStrategyRegistry::instance()->value("Mitchell"));
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
    return createPaintLayer(name, opacity, m_image->colorSpace()->colorModelId().id(), m_image->colorSpace()->id());
}

QObject* Image::createPaintLayer(const QString& name, int opacity, const QString& colorModelId, const QString& colorDepthId)
{
    opacity = CLAMP(opacity, 0, 255);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(colorModelId, colorDepthId, 0);
    KisPaintLayerSP layer = cs ? new KisPaintLayer(m_image.data(), name, opacity, cs)
                            : new KisPaintLayer(m_image.data(), name, opacity);
    layer->setVisible(true);
    m_image->addNode(layer.data(), m_image->rootLayer().data());
    return new PaintLayer(layer, m_doc);
}

#include "krs_image.moc"
