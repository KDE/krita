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

#include "kis_svg_brush.h"

#include <QDomElement>
#include <QFileInfo>
#include <QPainter>
#include <QImageReader>
#include <QSvgRenderer>

KisSvgBrush::KisSvgBrush(const QString& filename)
    : KisScalingSizeBrush(filename)
{
    setBrushType(INVALID);
    setSpacing(0.25);
    setHasColor(false);

}

KisSvgBrush::KisSvgBrush(const KisSvgBrush& rhs)
    : KisScalingSizeBrush(rhs)
    , m_svg(rhs.m_svg)
{
}

KisBrushSP KisSvgBrush::clone() const
{
    return KisBrushSP(new KisSvgBrush(*this));
}

bool KisSvgBrush::load()
{
    QFile f(filename());
    if (f.size() == 0) return false;
    if (!f.exists()) return false;
    if (!f.open(QIODevice::ReadOnly)) {
        warnKrita << "Can't open file " << filename();
        return false;
    }
    bool res = loadFromDevice(&f);
    f.close();

    return res;
}

bool KisSvgBrush::loadFromDevice(QIODevice *dev)
{

    m_svg = dev->readAll();

    QSvgRenderer renderer(m_svg);

    QRect box = renderer.viewBox();
    if (box.isEmpty()) return false;

    QImage image_(1000, (1000 * box.height()) / box.width(), QImage::Format_ARGB32);
    {
        QPainter p(&image_);
        p.fillRect(0, 0, image_.width(), image_.height(), Qt::white);
        renderer.render(&p);
    }

    QVector<QRgb> table;
    for (int i = 0; i < 256; ++i) table.push_back(qRgb(i, i, i));
    image_ = image_.convertToFormat(QImage::Format_Indexed8, table);

    setBrushTipImage(image_);

    setValid(true);

    // Well for now, always true
    if (brushTipImage().isGrayscale()) {
        setBrushType(MASK);
        setHasColor(false);
    }
    else {
        setBrushType(IMAGE);
        setHasColor(true);
    }
    setWidth(brushTipImage().width());
    setHeight(brushTipImage().height());

    QFileInfo fi(filename());
    setName(fi.baseName());

    return !brushTipImage().isNull() && valid();
}

bool KisSvgBrush::save()
{
    QFile f(filename());
    if (!f.open(QFile::WriteOnly)) return false;
    bool res = saveToDevice(&f);
    f.close();
    return res;
}

bool KisSvgBrush::saveToDevice(QIODevice *dev) const
{
    if((dev->write(m_svg.constData(), m_svg.size()) == m_svg.size())) {
        KoResource::saveToDevice(dev);
        return true;
    }
    return false;
}

QString KisSvgBrush::defaultFileExtension() const
{
    return QString(".svg");
}

void KisSvgBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    predefinedBrushToXML("svg_brush", e);
    KisBrush::toXML(d, e);
}
