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
#include <QByteArray>
#include <QBuffer>

#include <kis_dom_utils.h>

KisPngBrush::KisPngBrush(const QString& filename)
    : KisScalingSizeBrush(filename)
{
    setBrushType(INVALID);
    setSpacing(0.25);
    setHasColor(false);
}

KisPngBrush::KisPngBrush(const KisPngBrush &rhs)
    : KisScalingSizeBrush(rhs)
{
    setSpacing(rhs.spacing());
    if (brushTipImage().isGrayscale()) {
        setBrushType(MASK);
        setHasColor(false);
    }
    else {
        setBrushType(IMAGE);
        setHasColor(true);
    }
}

KisBrush* KisPngBrush::clone() const
{
    return new KisPngBrush(*this);
}

bool KisPngBrush::load()
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

bool KisPngBrush::loadFromDevice(QIODevice *dev)
{

    // Workaround for some OS (Debian, Ubuntu), where loading directly from the QIODevice
    // fails with "libpng error: IDAT: CRC error"
    QByteArray data = dev->readAll();
    QBuffer buf(&data);
    buf.open(QIODevice::ReadOnly);
    QImageReader reader(&buf, "PNG");

    if (!reader.canRead()) {
        dbgKrita << "Could not read brush" << filename() << ". Error:" << reader.errorString();
        setValid(false);
        return false;
    }

    if (reader.textKeys().contains("brush_spacing")) {
        setSpacing(KisDomUtils::toDouble(reader.text("brush_spacing")));
    }

    if (reader.textKeys().contains("brush_name")) {
        setName(reader.text("brush_name"));
    }
    else {
        QFileInfo info(filename());
        setName(info.baseName());
    }

    QImage image = reader.read();

    if (image.isNull()) {
        dbgKrita << "Could not create image for" << filename() << ". Error:" << reader.errorString();
        setValid(false);
        return false;
    }

    setValid(true);

    if (brushTipImage().allGray()) {
        setBrushTipImage(image.convertToFormat(QImage::Format_Grayscale8));
        setBrushType(MASK);
        setHasColor(false);
    }
    else {
        setBrushTipImage(image);
        setBrushType(IMAGE);
        setHasColor(true);
    }

    setWidth(brushTipImage().width());
    setHeight(brushTipImage().height());

    return valid();
}

bool KisPngBrush::save()
{
    QFile f(filename());
    if (!f.open(QFile::WriteOnly)) return false;
    bool res = saveToDevice(&f);
    f.close();
    return res;
}

bool KisPngBrush::saveToDevice(QIODevice *dev) const
{
    if(brushTipImage().save(dev, "PNG")) {
        KoResource::saveToDevice(dev);
        return true;
    }

    return false;
}

QString KisPngBrush::defaultFileExtension() const
{
    return QString(".png");
}

void KisPngBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    predefinedBrushToXML("png_brush", e);
    KisBrush::toXML(d, e);
}
