/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
}

KisSvgBrush::KisSvgBrush(const KisSvgBrush& rhs)
    : KisScalingSizeBrush(rhs)
    , m_svg(rhs.m_svg)
{
}

KoResourceSP KisSvgBrush::clone() const
{
    return KoResourceSP(new KisSvgBrush(*this));
}

bool KisSvgBrush::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

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
    }
    else {
        setBrushType(IMAGE);
    }
    setWidth(brushTipImage().width());
    setHeight(brushTipImage().height());

    QFileInfo fi(filename());
    setName(fi.completeBaseName());

    return !brushTipImage().isNull() && valid();
}

bool KisSvgBrush::saveToDevice(QIODevice *dev) const
{
    return dev->write(m_svg.constData(), m_svg.size()) == m_svg.size();
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
