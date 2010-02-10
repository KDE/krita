/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2007 Eric Lamarque <eric.lamarque@free.fr>
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
#include "kis_abr_brush.h"
#include "kis_brush.h"
#include "kis_qimage_mask.h"

#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPoint>

#include <kis_debug.h>
#include <klocale.h>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_iterators_pixel.h"
#include "kis_image.h"

#define DEFAULT_SPACING 0.25

KisAbrBrush::KisAbrBrush(const QString& filename)
    : KisBrush(filename)
{
    setBrushType(INVALID);
    setHasColor(false);
    setSpacing(DEFAULT_SPACING);
}

bool KisAbrBrush::load()
{
    return false;
}

bool KisAbrBrush::save()
{
    return false;
}

bool KisAbrBrush::saveToDevice(QIODevice* dev) const
{
    return false;
}

QImage KisAbrBrush::image() const
{
    return m_image;
}


enumBrushType KisAbrBrush::brushType() const
{
    return MASK;
}


void KisAbrBrush::setImage(const QImage& image)
{
    KisBrush::setImage(image);
    setValid(true);
}

void KisAbrBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    Q_UNUSED(d);
    e.setAttribute("type", "brush"); // legacy
    e.setAttribute("brush_type", "kis_abr_brush");
//    e.setAttribute("name", name()); // legacy
//    e.setAttribute("filename", filename()); // legacy
//    e.setAttribute("brush_filename", filename());
//    e.setAttribute("brush_spacing", spacing());
}

QString KisAbrBrush::defaultFileExtension() const
{
    return QString::null;
}
