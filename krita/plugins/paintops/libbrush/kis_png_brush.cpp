/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_png_brush.h"

#include <QDomElement>
#include <QFileInfo>
#include <QImageReader>

struct KisPngBrush::Private {
};

KisPngBrush::KisPngBrush(const QString& filename) : KisBrush(filename), d(new Private)
{
    setBrushType(INVALID);
    setSpacing(0.25);
    setHasColor(false);
    
}

bool KisPngBrush::load()
{
    QImageReader reader(filename(), "PNG");
    if(reader.textKeys().contains("brush_spacing"))
    {
        setSpacing(reader.text("brush_spacing").toDouble());
    }
    if(reader.textKeys().contains("brush_name"))
    {
        setName(reader.text("brush_name"));
    } else {
        QFileInfo info(filename());
        setName(info.baseName());
    }
    setImage(reader.read());
    setValid(!image().isNull());
    if(image().isGrayscale())
    {
        setBrushType(MASK);
        setHasColor(false);
    } else {
        setBrushType(IMAGE);
        setHasColor(true);
    }
    setWidth(image().width());
    setHeight(image().height());
    return !image().isNull();
}

QString KisPngBrush::defaultFileExtension() const
{
    return QString(".png");
}

void KisPngBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    Q_UNUSED(d);
    e.setAttribute("type", "png_brush");
    e.setAttribute("filename", shortFilename());
    e.setAttribute("spacing", spacing());
    KisBrush::toXML(d, e);
}
