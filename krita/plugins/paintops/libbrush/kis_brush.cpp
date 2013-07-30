/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_brush.h"

#include <QDomElement>
#include <QFile>
#include <QPoint>

#include <kis_debug.h>
#include <klocale.h>

#include <KoColor.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_boundary.h"
#include "kis_image.h"
#include "kis_iterator_ng.h"
#include "kis_brush_registry.h"
#include <kis_paint_information.h>
#include <kis_fixed_paint_device.h>
#include <kis_qimage_pyramid.h>


KisBrush::ColoringInformation::~ColoringInformation()
{
}

KisBrush::PlainColoringInformation::PlainColoringInformation(const quint8* color) : m_color(color)
{
}

KisBrush::PlainColoringInformation::~PlainColoringInformation()
{
}

const quint8* KisBrush::PlainColoringInformation::color() const
{
    return m_color;
}

void KisBrush::PlainColoringInformation::nextColumn()
{
}

void KisBrush::PlainColoringInformation::nextRow()
{
}

KisBrush::PaintDeviceColoringInformation::PaintDeviceColoringInformation(const KisPaintDeviceSP source, int width)
    : m_source(source)
    , m_iterator(m_source->createHLineConstIteratorNG(0, 0, width))
{
}

KisBrush::PaintDeviceColoringInformation::~PaintDeviceColoringInformation()
{
}

const quint8* KisBrush::PaintDeviceColoringInformation::color() const
{
    return m_iterator->oldRawData();
}

void KisBrush::PaintDeviceColoringInformation::nextColumn()
{
    m_iterator->nextPixel();
}
void KisBrush::PaintDeviceColoringInformation::nextRow()
{
    m_iterator->nextRow();
}


struct KisBrush::Private {
    Private()
        : boundary(0)
        , angle(0)
        , scale(1.0)
        , hasColor(false)
        , brushType(INVALID)
        , brushPyramid(0)
    {}

    ~Private() {
        delete boundary;
    }

    mutable KisBoundary* boundary;
    qreal angle;
    qreal scale;
    bool hasColor;
    enumBrushType brushType;

    qint32 width;
    qint32 height;
    double spacing;
    QPointF hotSpot;

    mutable KisQImagePyramid *brushPyramid;
};

KisBrush::KisBrush()
    : KoResource("")
    , d(new Private)
{
}

KisBrush::KisBrush(const QString& filename)
    : KoResource(filename)
    , d(new Private)
{
}

KisBrush::KisBrush(const KisBrush& rhs)
    : KoResource("")
    , KisShared()
    , d(new Private)
{
    m_image = rhs.m_image;
    d->brushType = rhs.d->brushType;
    d->width = rhs.d->width;
    d->height = rhs.d->height;
    d->spacing = rhs.d->spacing;
    d->hotSpot = rhs.d->hotSpot;
    d->hasColor = rhs.d->hasColor;
    d->angle = rhs.d->angle;
    d->scale = rhs.d->scale;
    setFilename(rhs.filename());
    clearBrushPyramid();
    // don't copy the boundery, it will be regenerated -- see bug 291910
}

KisBrush::~KisBrush()
{
    clearBrushPyramid();
    delete d;
}

QImage KisBrush::image() const
{
    return m_image;
}

qint32 KisBrush::width() const
{
    return d->width;
}

void KisBrush::setWidth(qint32 width)
{
    d->width = width;
}

qint32 KisBrush::height() const
{
    return d->height;
}

void KisBrush::setHeight(qint32 height)
{
    d->height = height;
}

void KisBrush::setHotSpot(QPointF pt)
{
    double x = pt.x();
    double y = pt.y();

    if (x < 0)
        x = 0;
    else if (x >= width())
        x = width() - 1;

    if (y < 0)
        y = 0;
    else if (y >= height())
        y = height() - 1;

    d->hotSpot = QPointF(x, y);
}

QPointF KisBrush::hotSpot(double scaleX, double scaleY, double rotation, const KisPaintInformation& info) const
{
    Q_UNUSED(scaleY);

    double w = maskWidth(scaleX, rotation, info);
    double h = maskHeight(scaleX, rotation, info);

    // The smallest brush we can produce is a single pixel.
    if (w < 1) {
        w = 1;
    }

    if (h < 1) {
        h = 1;
    }

    // XXX: This should take d->hotSpot into account, though it
    // isn't specified by gimp brushes so it would default to the center
    // anyway.
    QPointF p(w / 2, h / 2);
    return p;
}


bool KisBrush::hasColor() const
{
    return d->hasColor;
}

void KisBrush::setHasColor(bool hasColor)
{
    d->hasColor = hasColor;
}


bool KisBrush::canPaintFor(const KisPaintInformation& /*info*/)
{
    return true;
}

void KisBrush::setImage(const QImage& image)
{
    Q_ASSERT(!image.isNull());
    m_image = image;

    setWidth(image.width());
    setHeight(image.height());

    clearBrushPyramid();

}

void KisBrush::setBrushType(enumBrushType type)
{
    d->brushType = type;
}

enumBrushType KisBrush::brushType() const
{
    return d->brushType;
}

void KisBrush::toXML(QDomDocument& /*document*/ , QDomElement& element) const
{
    element.setAttribute("BrushVersion", "2");
}

KisBrushSP KisBrush::fromXML(const QDomElement& element)
{
    KisBrushSP brush = KisBrushRegistry::instance()->getOrCreateBrush(element);
    if(brush && element.attribute("BrushVersion", "1") == "1")
    {
        brush->setScale(brush->scale() * 2.0);
    }
    return brush;
}

qint32 KisBrush::maskWidth(double scale, double angle, const KisPaintInformation& info) const
{
    Q_UNUSED(info);

    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scale *= d->scale;

    return KisQImagePyramid::imageSize(QSize(width(), height()),
                                       scale, angle, 0.5, 0.5).width();
}

qint32 KisBrush::maskHeight(double scale, double angle, const KisPaintInformation& info) const
{
    Q_UNUSED(info);

    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scale *= d->scale;

    return KisQImagePyramid::imageSize(QSize(width(), height()),
                                       scale, angle, 0.5, 0.5).height();
}

double KisBrush::maskAngle(double angle) const
{
    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0)      { angle += 2*M_PI; }
    if(angle > 2*M_PI) { angle -= 2*M_PI; }

    return angle;
}

quint32 KisBrush::brushIndex(const KisPaintInformation& info) const
{
    Q_UNUSED(info);
    return 0;
}

void KisBrush::setSpacing(double s)
{
    if (s < 0.02) s = 0.02;
    d->spacing = s;
}

double KisBrush::spacing() const
{
    return d->spacing;
}

void KisBrush::notifyCachedDabPainted() {
}

void KisBrush::prepareBrushPyramid() const
{
    if (!d->brushPyramid) {
        d->brushPyramid = new KisQImagePyramid(image());
    }
}

void KisBrush::clearBrushPyramid()
{
    delete d->brushPyramid;
    d->brushPyramid = 0;
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, double scaleX, double scaleY, double angle, const KisPaintInformation& info , double subPixelX, double subPixelY, qreal softnessFactor) const
{
    generateMaskAndApplyMaskOrCreateDab(dst, 0, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KoColor& color, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    PlainColoringInformation pci(color.data());
    generateMaskAndApplyMaskOrCreateDab(dst, &pci, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KisPaintDeviceSP src, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    PaintDeviceColoringInformation pdci(src, maskWidth(scaleX, angle, info));
    generateMaskAndApplyMaskOrCreateDab(dst, &pdci, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}


void KisBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
                                                   ColoringInformation* coloringInformation,
                                                   double scaleX, double scaleY, double angle,
                                                   const KisPaintInformation& info_,
                                                   double subPixelX, double subPixelY, qreal softnessFactor) const
{
    Q_ASSERT(valid());
    Q_UNUSED(info_);
    Q_UNUSED(softnessFactor);

    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scaleX *= d->scale;
    scaleY *= d->scale;

    double scale = 0.5 * (scaleX + scaleY);

    prepareBrushPyramid();
    QImage outputImage = d->brushPyramid->createImage(scale, -angle, subPixelX, subPixelY);

    qint32 maskWidth = outputImage.width();
    qint32 maskHeight = outputImage.height();

    dst->setRect(QRect(0, 0, maskWidth, maskHeight));
    dst->initialize();

    quint8* color = 0;

    if (coloringInformation) {
        if (dynamic_cast<PlainColoringInformation*>(coloringInformation)) {
            color = const_cast<quint8*>(coloringInformation->color());
        }
    }

    const KoColorSpace *cs = dst->colorSpace();
    qint32 pixelSize = cs->pixelSize();
    quint8 *dabPointer = dst->data();
    quint8 *rowPointer = dabPointer;
    quint8 *alphaArray = new quint8[maskWidth];
    bool hasColor = this->hasColor();

    for (int y = 0; y < maskHeight; y++) {
        const quint8* maskPointer = outputImage.constScanLine(y);
        if (coloringInformation) {
            for (int x = 0; x < maskWidth; x++) {
                if (color) {
                    memcpy(dabPointer, color, pixelSize);
                } else {
                    memcpy(dabPointer, coloringInformation->color(), pixelSize);
                    coloringInformation->nextColumn();
                }
                dabPointer += pixelSize;
            }
        }

        if (hasColor) {
            const quint8 *src = maskPointer;
            quint8 *dst = alphaArray;
            for (int x = 0; x < maskWidth; x++) {
                const QRgb *c = reinterpret_cast<const QRgb*>(src);

                *dst = KoColorSpaceMaths<quint8>::multiply(255 - qGray(*c), qAlpha(*c));
                src += 4;
                dst++;
            }
        } else {
            const quint8 *src = maskPointer;
            quint8 *dst = alphaArray;
            for (int x = 0; x < maskWidth; x++) {
                const QRgb *c = reinterpret_cast<const QRgb*>(src);

                *dst = KoColorSpaceMaths<quint8>::multiply(255 - *src, qAlpha(*c));
                src += 4;
                dst++;
            }
        }

        cs->applyAlphaU8Mask(rowPointer, alphaArray, maskWidth);
        rowPointer += maskWidth * pixelSize;
        dabPointer = rowPointer;

        if (!color && coloringInformation) {
            coloringInformation->nextRow();
        }
    }

    delete alphaArray;
}

KisFixedPaintDeviceSP KisBrush::paintDevice(const KoColorSpace * colorSpace,
                                            double scale, double angle,
                                            const KisPaintInformation& info,
                                            double subPixelX, double subPixelY) const
{
    Q_ASSERT(valid());
    Q_UNUSED(info);
    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scale *= d->scale;

    prepareBrushPyramid();
    QImage outputImage = d->brushPyramid->createImage(scale, -angle, subPixelX, subPixelY);

    KisFixedPaintDeviceSP dab = new KisFixedPaintDevice(colorSpace);
    Q_CHECK_PTR(dab);
    dab->convertFromQImage(outputImage, "");

    return dab;
}

void KisBrush::resetBoundary()
{
    delete d->boundary;
    d->boundary = 0;
}

void KisBrush::generateBoundary() const
{
    KisFixedPaintDeviceSP dev;

    if (brushType() == IMAGE || brushType() == PIPE_IMAGE) {
        dev = paintDevice(KoColorSpaceRegistry::instance()->rgb8(), 1.0/scale(), -angle(), KisPaintInformation());
    } else {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        dev = new KisFixedPaintDevice(cs);
        mask(dev, KoColor(Qt::black, cs) , 1.0/scale(), 1.0/scale(), -angle(), KisPaintInformation());
    }

    d->boundary = new KisBoundary(dev);
    d->boundary->generateBoundary();
}

const KisBoundary* KisBrush::boundary() const
{
    if (!d->boundary)
        generateBoundary();
    return d->boundary;
}

void KisBrush::setScale(qreal _scale)
{
  d->scale = _scale;
}

qreal KisBrush::scale() const
{
  return d->scale;
}

void KisBrush::setAngle(qreal _rotation)
{
  d->angle = _rotation;
}

qreal KisBrush::angle() const
{
  return d->angle;
}

QPainterPath KisBrush::outline() const
{
    return boundary()->path();
}
