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
    return true;
}

bool KisAbrBrush::save()
{
    //Return true, otherwise the brush won't be add to the
    //resource server if the brush is loaded via import
    return true;
}

bool KisAbrBrush::saveToDevice(QIODevice* dev) const
{
    Q_UNUSED(dev);
    return false;
}

void KisAbrBrush::setImage(const QImage& image)
{
    setValid(true);
    setBrushType(MASK);
    setHasColor(false);
    KisBrush::setImage(image);
}

void KisAbrBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    Q_UNUSED(d);
    e.setAttribute("type", "abr_brush");
    e.setAttribute("name", name()); // legacy
    e.setAttribute("filename", shortFilename());
    e.setAttribute("spacing", spacing());
    KisBrush::toXML(d, e);
}

QString KisAbrBrush::defaultFileExtension() const
{
    return QString::null;
}
