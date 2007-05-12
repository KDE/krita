/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "krs_paint_layer.h"

#include <KoColorSpaceRegistry.h>
#include <kis_doc2.h>
#include <kis_layer.h>

#include "krs_image.h"
#include "krs_paint_device.h"

using namespace Scripting;

PaintLayer::PaintLayer(Image* image, KisPaintLayerSP layer, KisDoc2* doc)
    : QObject(image)
    , m_image(image)
    , m_layer(layer)
    , m_doc(doc)
    , m_cmd(0)
{
    setObjectName("KritaLayer");
}


PaintLayer::~PaintLayer()
{
}

QObject* PaintLayer::paintDevice()
{
    return new PaintDevice(m_image, paintLayer()->paintDevice(), m_doc);
}

#include "krs_paint_layer.moc"
