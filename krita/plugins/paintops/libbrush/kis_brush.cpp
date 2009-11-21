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
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_boundary.h"
#include "kis_iterators_pixel.h"
#include "kis_image.h"
#include "kis_scaled_brush.h"
#include "kis_qimage_mask.h"

#include "kis_brush_registry.h"

#define MAXIMUM_SCALE 2

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

KisBrush::PaintDeviceColoringInformation::PaintDeviceColoringInformation(const KisPaintDeviceSP source, int width) : m_source(source), m_iterator(new KisHLineConstIteratorPixel(m_source->createHLineConstIterator(0, 0, width)))
{
}

KisBrush::PaintDeviceColoringInformation::~PaintDeviceColoringInformation()
{
    delete m_iterator;
}

const quint8* KisBrush::PaintDeviceColoringInformation::color() const
{
    return m_iterator->oldRawData();
}

void KisBrush::PaintDeviceColoringInformation::nextColumn()
{
    ++(*m_iterator);
}
void KisBrush::PaintDeviceColoringInformation::nextRow()
{
    m_iterator->nextRow();
}


struct KisBrush::Private {
    Private() : boundary(0) {}
    ~Private() {
        delete boundary;
    }
    enumBrushType brushType;
    qint32 width;
    qint32 height;
    double spacing;
    QPointF hotSpot;
    mutable QVector<KisScaledBrush> scaledBrushes;
    bool hasColor;
    mutable KisBoundary* boundary;

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
        , d(new Private)
{
    m_image = rhs.m_image;
    d->brushType = rhs.d->brushType;
    d->width = rhs.d->width;
    d->height = rhs.d->height;
    d->scaledBrushes.clear();
    setFilename(rhs.filename());
}

KisBrush::~KisBrush()
{
    clearScaledBrushes();
    delete d;
}

QImage KisBrush::img() const
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

QPointF KisBrush::hotSpot(double scaleX, double scaleY, double rotation) const
{
    double w = maskWidth( scaleX, rotation);
    double h = maskHeight( scaleX, rotation);

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

void KisBrush::setImage(const QImage& img)
{
    Q_ASSERT(!img.isNull());
    m_image = img;

    setWidth(img.width());
    setHeight(img.height());

    clearScaledBrushes();

}

void KisBrush::setBrushType(enumBrushType type)
{
    d->brushType = type;
}

enumBrushType KisBrush::brushType() const
{
    return d->brushType;
}

KisBrushSP KisBrush::fromXML(const QDomElement& element)
{

    return KisBrushRegistry::instance()->getOrCreateBrush(element);

}

qint32 KisBrush::maskWidth(double scale, double angle) const
{
    double width_ = width() * scale;
    if(angle == 0.0) return qint32(width_ + 1);
    
    double height_ = height() * scale;
    
    // Add one for sub-pixel shift
    if (angle >= 0.0 && angle < M_PI_2) {
        return qAbs(static_cast<qint32>(ceil(width_ * cos(angle) + height_ * sin(angle)) + 1));
    } else if (angle >= M_PI_2 && angle < M_PI) {
        return qAbs(static_cast<qint32>(ceil(-width_ * cos(angle) + height_ * sin(angle)) + 1));
    } else if (angle >= M_PI && angle < (M_PI + M_PI_2)) {
        return qAbs(static_cast<qint32>(ceil(-width_ * cos(angle) - height_ * sin(angle)) + 1));
    } else {
        return qAbs(static_cast<qint32>(ceil(width_ * cos(angle) - height_ * sin(angle)) + 1));
    }
}

qint32 KisBrush::maskHeight(double scale, double angle) const
{
    double height_ = height() * scale;
    if(angle == 0.0) return qint32(height_ + 1);
    
    double width_ = width() * scale;
    
    // Add one for sub-pixel shift
    if (angle >= 0.0 && angle < M_PI_2) {
        return qAbs(static_cast<qint32>(ceil(width_ * sin(angle) + height_ * cos(angle)) + 1));
    } else if (angle >= M_PI_2 && angle < M_PI) {
        return qAbs(static_cast<qint32>(ceil(width_ * sin(angle) - height_ * cos(angle)) + 1));
    } else if (angle >= M_PI && angle < (M_PI + M_PI_2)) {
        return qAbs(static_cast<qint32>(ceil(-width_ * sin(angle) - height_ * cos(angle)) + 1));
    } else {
        return qAbs(static_cast<qint32>(ceil(-width_ * sin(angle) + height_ * cos(angle)) + 1));
    }
}

double KisBrush::xSpacing(double scale) const
{
    return width() * scale * d->spacing;
}

double KisBrush::ySpacing(double scale) const
{
    return height() * scale * d->spacing;
}

void KisBrush::setSpacing(double s)
{
    d->spacing = s;
}

double KisBrush::spacing() const
{
    return d->spacing;
}
void KisBrush::mask(KisFixedPaintDeviceSP dst, double scaleX, double scaleY, double angle, const KisPaintInformation& info , double subPixelX, double subPixelY) const
{
    generateMaskAndApplyMaskOrCreateDab(dst, 0, scaleX, scaleY, angle, info, subPixelX, subPixelY);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KoColor& color, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    PlainColoringInformation pci(color.data());
    generateMaskAndApplyMaskOrCreateDab(dst, &pci, scaleX, scaleY, angle, info, subPixelX, subPixelY);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KisPaintDeviceSP src, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    PaintDeviceColoringInformation pdci(src, maskWidth(scaleX, angle));
    generateMaskAndApplyMaskOrCreateDab(dst, &pdci, scaleX, scaleY, angle, info, subPixelX, subPixelY);
}


void KisBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
        ColoringInformation* coloringInformation,
        double scaleX, double scaleY, double angle,
        const KisPaintInformation& info_,
        double subPixelX, double subPixelY) const
{
    Q_ASSERT(valid());
    Q_UNUSED(angle);
    Q_UNUSED(info_);

    const KoColorSpace* cs = dst->colorSpace();
    quint32 pixelSize = cs->pixelSize();

    double scale = 0.5 * (scaleX + scaleY);

    KisQImagemaskSP outputMask = createMask(scale, subPixelX, subPixelY);

    qint32 maskWidth = outputMask->width();
    qint32 maskHeight = outputMask->height();

    if (coloringInformation) {

        // old bounds
        QRect bounds = dst->bounds();

        // new bounds. we don't care if there is some extra memory occcupied.
        dst->setRect(QRect(0, 0, maskWidth, maskHeight));

        if (maskWidth * maskHeight <= bounds.width() * bounds.height()) {
            // just clear the data in dst,
            memset(dst->data(), OPACITY_TRANSPARENT, maskWidth * maskHeight * dst->pixelSize());
        } else {
            dst->initialize();
        }
    } else {
        if (dst->data() == 0 || dst->bounds().isEmpty()) {
            qWarning() << "Creating a default black dab: no coloring info and no initialized paint device to mask";
            dst->clear(QRect(0, 0, maskWidth, maskHeight));
        }
    }
    Q_ASSERT(dst->bounds().size().width() >= maskWidth && dst->bounds().size().height() >= maskHeight);

    quint8* dabPointer = dst->data();
    quint8* color = 0;

    if (coloringInformation) {
        if (dynamic_cast<PlainColoringInformation*>(coloringInformation)) {
            color = const_cast<quint8*>(coloringInformation->color());
        }
    } else {
        // Mask everything out
        cs->setAlpha(dst->data(), OPACITY_TRANSPARENT, dst->bounds().width() * dst->bounds().height());
    }

    int rowWidth = dst->bounds().width();

    quint8* maskPointer = outputMask->data();
    quint8* rowPointer = dabPointer;

    for (int y = 0; y < maskHeight; y++) {
        for (int x = 0; x < maskWidth; x++) {
            if (coloringInformation) {
                if (color) {
                    memcpy(dabPointer, color, pixelSize);
                } else {
                    memcpy(dabPointer, coloringInformation->color(), pixelSize);
                    coloringInformation->nextColumn();
                }
            }

            dabPointer += pixelSize;
        }
        cs->applyAlphaU8Mask(rowPointer, maskPointer, maskWidth);
        maskPointer += maskWidth;
        rowPointer += maskWidth * pixelSize;

        if (!color && coloringInformation) {
            coloringInformation->nextRow();
        }
        if (maskWidth < rowWidth) {
            dabPointer += (pixelSize * (rowWidth - maskWidth));
        }
    }
}

KisFixedPaintDeviceSP KisBrush::image(const KoColorSpace * colorSpace,
                                      double scale, double angle,
                                      const KisPaintInformation& info,
                                      double subPixelX, double subPixelY) const
{
    Q_ASSERT(valid());
    Q_UNUSED(colorSpace);
    Q_UNUSED(info);
    Q_UNUSED(angle);
    if (d->scaledBrushes.isEmpty()) {
        createScaledBrushes();
    }

    const KisScaledBrush *aboveBrush = 0;
    const KisScaledBrush *belowBrush = 0;

    findScaledBrushes(scale, &aboveBrush, &belowBrush);
    Q_ASSERT(aboveBrush != 0);

    QImage outputImage;

    if (belowBrush != 0) {
        // We're in between two brushes. Interpolate between them.

        QImage scaledAboveImage = scaleImage(aboveBrush, scale, subPixelX, subPixelY);

        QImage scaledBelowImage = scaleImage(belowBrush, scale, subPixelX, subPixelY);

        double t = (scale - belowBrush->scale()) / (aboveBrush->scale() - belowBrush->scale());

        outputImage = interpolate(scaledBelowImage, scaledAboveImage, t);

    } else {
        if (Eigen::ei_isApprox(scale, aboveBrush->scale())) {
            // Exact match.
            outputImage = scaleImage(aboveBrush, scale, subPixelX, subPixelY);

        } else {
            // We are smaller than the smallest brush, which is always 1x1.
            double s = scale / aboveBrush->scale();
            outputImage = scaleSinglePixelImage(s, aboveBrush->image().pixel(0, 0), subPixelX, subPixelY);

        }
    }

    int outputWidth = outputImage.width();
    int outputHeight = outputImage.height();

    KisFixedPaintDeviceSP dab = new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    Q_CHECK_PTR(dab);
    dab->setRect(outputImage.rect());
    dab->initialize();
    quint8* dabPointer = dab->data();
    quint32 pixelSize = dab->pixelSize();

    for (int y = 0; y < outputHeight; y++) {
        const QRgb *scanline = reinterpret_cast<const QRgb *>(outputImage.scanLine(y));
        for (int x = 0; x < outputWidth; x++) {
            QRgb pixel = scanline[x];

            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);
            int alpha = qAlpha(pixel);

            // Scaled images are in pre-multiplied alpha form so
            // divide by alpha.
            // XXX: Is alpha != 0 ever true?
            // channel order is BGRA
            if (alpha != 0) {
                dabPointer[2] = (red * 255) / alpha;
                dabPointer[1] = (green * 255) / alpha;
                dabPointer[0] = (blue * 255) / alpha;
                dabPointer[3] = alpha;
            } else {
                dabPointer[2] = red;
                dabPointer[1] = green;
                dabPointer[0] = blue;
                dabPointer[3] = 0;
            }

            dabPointer += pixelSize;

        }
    }
    return dab;
}

void KisBrush::clearScaledBrushes()
{
    d->scaledBrushes.clear();
}

void KisBrush::createScaledBrushes() const
{
    if (!d->scaledBrushes.isEmpty()) {
        const_cast<KisBrush*>(this)->clearScaledBrushes();
    }

    if (img().isNull()) {
        return;
    }
    // Construct a series of brushes where each one's dimensions are
    // half the size of the previous one.
    int width = img().width() * MAXIMUM_SCALE;
    int height = img().height() * MAXIMUM_SCALE;

    QImage scaledImage;
    while (true) {

        if (width >= img().width() && height >= img().height()) {
            scaledImage = scaleImage(img(), width, height);
        } else {
            // Scale down the previous image once we're below 1:1.
            scaledImage = scaleImage(scaledImage, width, height);
        }

        KisQImagemaskSP scaledMask = KisQImagemaskSP(new KisQImagemask(scaledImage, hasColor()));
        Q_CHECK_PTR(scaledMask);

        double xScale = static_cast<double>(width) / img().width();
        double yScale = static_cast<double>(height) / img().height();
        double scale = xScale;

        d->scaledBrushes.append(KisScaledBrush(scaledMask, hasColor() ? scaledImage : QImage(), scale, xScale, yScale));

        if (width == 1 && height == 1) {
            break;
        }

        // Round up so that we never have to scale an image by less than 1/2.
        width = (width + 1) / 2;
        height = (height + 1) / 2;

    }
}

KisQImagemaskSP KisBrush::createMask(double scale, double subPixelX, double subPixelY) const
{
    if (d->scaledBrushes.isEmpty()) {
        createScaledBrushes();
    }

    const KisScaledBrush *aboveBrush = 0;
    const KisScaledBrush *belowBrush = 0;

    findScaledBrushes(scale, &aboveBrush,  &belowBrush);
    Q_ASSERT(aboveBrush != 0);

    // get the right mask
    KisQImagemaskSP outputMask = KisQImagemaskSP(0);

    if (belowBrush != 0) {
        // We're in between two masks. Interpolate between them.

        KisQImagemaskSP scaledAboveMask = scaleMask(aboveBrush, scale, subPixelX, subPixelY);
        KisQImagemaskSP scaledBelowMask = scaleMask(belowBrush, scale, subPixelX, subPixelY);

        double t = (scale - belowBrush->scale()) / (aboveBrush->scale() - belowBrush->scale());

        outputMask = KisQImagemask::interpolate(scaledBelowMask, scaledAboveMask, t);
    } else {
        if (Eigen::ei_isApprox(scale, aboveBrush->scale())) {
            // Exact match.
            outputMask = scaleMask(aboveBrush, scale, subPixelX, subPixelY);
        } else {
            // We are smaller than the smallest mask, which is always 1x1.
            double s = scale / aboveBrush->scale();
            outputMask = scaleSinglePixelMask(s, aboveBrush->mask()->alphaAt(0, 0), subPixelX, subPixelY);
        }
    }

    return outputMask;
}

KisQImagemaskSP KisBrush::scaleMask(const KisScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const
{
    // Add one pixel for sub-pixel shifting
    int dstWidth = static_cast<int>(ceil(scale * width())) + 1;
    int dstHeight = static_cast<int>(ceil(scale * height())) + 1;

    KisQImagemaskSP dstMask = KisQImagemaskSP(new KisQImagemask(dstWidth, dstHeight));
    Q_CHECK_PTR(dstMask);

    KisQImagemaskSP srcMask = srcBrush->mask();

    // Compute scales to map the scaled brush onto the required scale.
    double xScale = srcBrush->xScale() / scale;
    double yScale = srcBrush->yScale() / scale;

    int srcWidth = srcMask->width();
    int srcHeight = srcMask->height();

    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {

            double srcX = (dstX - subPixelX + 0.5) * xScale;
            double srcY = (dstY - subPixelY + 0.5) * yScale;

            srcX -= 0.5;
            srcY -= 0.5;

            int leftX = static_cast<int>(srcX);

            if (srcX < 0) {
                leftX--;
            }

            double xInterp = srcX - leftX;

            int topY = static_cast<int>(srcY);

            if (srcY < 0) {
                topY--;
            }

            double yInterp = srcY - topY;

            quint8 topLeft = (leftX >= 0 && leftX < srcWidth && topY >= 0 && topY < srcHeight) ? srcMask->alphaAt(leftX, topY) : OPACITY_TRANSPARENT;
            quint8 bottomLeft = (leftX >= 0 && leftX < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcMask->alphaAt(leftX, topY + 1) : OPACITY_TRANSPARENT;
            quint8 topRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY >= 0 && topY < srcHeight) ? srcMask->alphaAt(leftX + 1, topY) : OPACITY_TRANSPARENT;
            quint8 bottomRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcMask->alphaAt(leftX + 1, topY + 1) : OPACITY_TRANSPARENT;

            double a = 1 - xInterp;
            double b = 1 - yInterp;

            // Bi-linear interpolation
            int d = static_cast<int>(a * b * topLeft
                                     + a * (1 - b) * bottomLeft
                                     + (1 - a) * b * topRight
                                     + (1 - a) * (1 - b) * bottomRight + 0.5);

            if (d < OPACITY_TRANSPARENT) {
                d = OPACITY_TRANSPARENT;
            } else if (d > OPACITY_OPAQUE) {
                d = OPACITY_OPAQUE;
            }

            dstMask->setAlphaAt(dstX, dstY, static_cast<quint8>(d));
        }
    }

    return dstMask;
}

QImage KisBrush::scaleImage(const KisScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const
{
    // Add one pixel for sub-pixel shifting
    int dstWidth = static_cast<int>(ceil(scale * width())) + 1;
    int dstHeight = static_cast<int>(ceil(scale * height())) + 1;

    QImage dstImage(dstWidth, dstHeight, QImage::Format_ARGB32);

    const QImage srcImage = srcBrush->image();

    // Compute scales to map the scaled brush onto the required scale.
    double xScale = srcBrush->xScale() / scale;
    double yScale = srcBrush->yScale() / scale;

    int srcWidth = srcImage.width();
    int srcHeight = srcImage.height();

    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {

            double srcX = (dstX - subPixelX + 0.5) * xScale;
            double srcY = (dstY - subPixelY + 0.5) * yScale;

            srcX -= 0.5;
            srcY -= 0.5;

            int leftX = static_cast<int>(srcX);

            if (srcX < 0) {
                leftX--;
            }

            double xInterp = srcX - leftX;

            int topY = static_cast<int>(srcY);

            if (srcY < 0) {
                topY--;
            }

            double yInterp = srcY - topY;

            QRgb topLeft = (leftX >= 0 && leftX < srcWidth && topY >= 0 && topY < srcHeight) ? srcImage.pixel(leftX, topY) : qRgba(0, 0, 0, 0);
            QRgb bottomLeft = (leftX >= 0 && leftX < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcImage.pixel(leftX, topY + 1) : qRgba(0, 0, 0, 0);
            QRgb topRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY >= 0 && topY < srcHeight) ? srcImage.pixel(leftX + 1, topY) : qRgba(0, 0, 0, 0);
            QRgb bottomRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcImage.pixel(leftX + 1, topY + 1) : qRgba(0, 0, 0, 0);

            double a = 1 - xInterp;
            double b = 1 - yInterp;

            // Bi-linear interpolation. Image is pre-multiplied by alpha.
            int red = static_cast<int>(a * b * qRed(topLeft)
                                       + a * (1 - b) * qRed(bottomLeft)
                                       + (1 - a) * b * qRed(topRight)
                                       + (1 - a) * (1 - b) * qRed(bottomRight) + 0.5);
            int green = static_cast<int>(a * b * qGreen(topLeft)
                                         + a * (1 - b) * qGreen(bottomLeft)
                                         + (1 - a) * b * qGreen(topRight)
                                         + (1 - a) * (1 - b) * qGreen(bottomRight) + 0.5);
            int blue = static_cast<int>(a * b * qBlue(topLeft)
                                        + a * (1 - b) * qBlue(bottomLeft)
                                        + (1 - a) * b * qBlue(topRight)
                                        + (1 - a) * (1 - b) * qBlue(bottomRight) + 0.5);
            int alpha = static_cast<int>(a * b * qAlpha(topLeft)
                                         + a * (1 - b) * qAlpha(bottomLeft)
                                         + (1 - a) * b * qAlpha(topRight)
                                         + (1 - a) * (1 - b) * qAlpha(bottomRight) + 0.5);

            if (red < 0) {
                red = 0;
            } else if (red > 255) {
                red = 255;
            }

            if (green < 0) {
                green = 0;
            } else if (green > 255) {
                green = 255;
            }

            if (blue < 0) {
                blue = 0;
            } else if (blue > 255) {
                blue = 255;
            }

            if (alpha < 0) {
                alpha = 0;
            } else if (alpha > 255) {
                alpha = 255;
            }

            dstImage.setPixel(dstX, dstY, qRgba(red, green, blue, alpha));
        }
    }

    return dstImage;
}

QImage KisBrush::scaleImage(const QImage& srcImage, int width, int height)
{
    QImage scaledImage;
    //QString filename;

    int srcWidth = srcImage.width();
    int srcHeight = srcImage.height();

    double xScale = static_cast<double>(srcWidth) / width;
    double yScale = static_cast<double>(srcHeight) / height;

    if (xScale > 2 || yScale > 2 || xScale < 1 || yScale < 1) {
        // smoothScale gives better results when scaling an image up
        // or scaling it to less than half size.
        scaledImage = srcImage.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        //filename = QString("smoothScale_%1x%2.png").arg(width).arg(height);
    } else {
        scaledImage = QImage(width, height, srcImage.format());

        for (int dstY = 0; dstY < height; dstY++) {
            for (int dstX = 0; dstX < width; dstX++) {

                double srcX = (dstX + 0.5) * xScale;
                double srcY = (dstY + 0.5) * yScale;

                srcX -= 0.5;
                srcY -= 0.5;

                int leftX = static_cast<int>(srcX);

                if (srcX < 0) {
                    leftX--;
                }

                double xInterp = srcX - leftX;

                int topY = static_cast<int>(srcY);

                if (srcY < 0) {
                    topY--;
                }

                double yInterp = srcY - topY;

                QRgb topLeft = (leftX >= 0 && leftX < srcWidth && topY >= 0 && topY < srcHeight) ? srcImage.pixel(leftX, topY) : qRgba(0, 0, 0, 0);
                QRgb bottomLeft = (leftX >= 0 && leftX < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcImage.pixel(leftX, topY + 1) : qRgba(0, 0, 0, 0);
                QRgb topRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY >= 0 && topY < srcHeight) ? srcImage.pixel(leftX + 1, topY) : qRgba(0, 0, 0, 0);
                QRgb bottomRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcImage.pixel(leftX + 1, topY + 1) : qRgba(0, 0, 0, 0);

                double a = 1 - xInterp;
                double b = 1 - yInterp;

                int red;
                int green;
                int blue;
                int alpha;

                if (srcImage.hasAlphaChannel()) {
                    red = static_cast<int>(a * b * qRed(topLeft)         * qAlpha(topLeft)
                                           + a * (1 - b) * qRed(bottomLeft)             * qAlpha(bottomLeft)
                                           + (1 - a) * b * qRed(topRight)               * qAlpha(topRight)
                                           + (1 - a) * (1 - b) * qRed(bottomRight)      * qAlpha(bottomRight) + 0.5);
                    green = static_cast<int>(a * b * qGreen(topLeft)     * qAlpha(topLeft)
                                             + a * (1 - b) * qGreen(bottomLeft)           * qAlpha(bottomLeft)
                                             + (1 - a) * b * qGreen(topRight)             * qAlpha(topRight)
                                             + (1 - a) * (1 - b) * qGreen(bottomRight)    * qAlpha(bottomRight) + 0.5);
                    blue = static_cast<int>(a * b * qBlue(topLeft)       * qAlpha(topLeft)
                                            + a * (1 - b) * qBlue(bottomLeft)            * qAlpha(bottomLeft)
                                            + (1 - a) * b * qBlue(topRight)              * qAlpha(topRight)
                                            + (1 - a) * (1 - b) * qBlue(bottomRight)     * qAlpha(bottomRight) + 0.5);
                    alpha = static_cast<int>(a * b * qAlpha(topLeft)
                                             + a * (1 - b) * qAlpha(bottomLeft)
                                             + (1 - a) * b * qAlpha(topRight)
                                             + (1 - a) * (1 - b) * qAlpha(bottomRight) + 0.5);

                    if (alpha != 0) {
                        red /= alpha;
                        green /= alpha;
                        blue /= alpha;
                    }
                } else {
                    red = static_cast<int>(a * b * qRed(topLeft)
                                           + a * (1 - b) * qRed(bottomLeft)
                                           + (1 - a) * b * qRed(topRight)
                                           + (1 - a) * (1 - b) * qRed(bottomRight) + 0.5);
                    green = static_cast<int>(a * b * qGreen(topLeft)
                                             + a * (1 - b) * qGreen(bottomLeft)
                                             + (1 - a) * b * qGreen(topRight)
                                             + (1 - a) * (1 - b) * qGreen(bottomRight) + 0.5);
                    blue = static_cast<int>(a * b * qBlue(topLeft)
                                            + a * (1 - b) * qBlue(bottomLeft)
                                            + (1 - a) * b * qBlue(topRight)
                                            + (1 - a) * (1 - b) * qBlue(bottomRight) + 0.5);
                    alpha = 255;
                }

                if (red < 0) {
                    red = 0;
                } else if (red > 255) {
                    red = 255;
                }

                if (green < 0) {
                    green = 0;
                } else if (green > 255) {
                    green = 255;
                }

                if (blue < 0) {
                    blue = 0;
                } else if (blue > 255) {
                    blue = 255;
                }

                if (alpha < 0) {
                    alpha = 0;
                } else if (alpha > 255) {
                    alpha = 255;
                }

                scaledImage.setPixel(dstX, dstY, qRgba(red, green, blue, alpha));
            }
        }

        //filename = QString("bilinear_%1x%2.png").arg(width).arg(height);
    }

    //scaledImage.save(filename, "PNG");

    return scaledImage;
}

void KisBrush::findScaledBrushes(double scale, const KisScaledBrush **aboveBrush, const KisScaledBrush **belowBrush) const
{
    int current = 0;

    while (true) {
        *aboveBrush = &(d->scaledBrushes[current]);

        if (Eigen::ei_isApprox(scale, (*aboveBrush)->scale())) {
            // Scale matches exactly
            break;
        }

        if (current == d->scaledBrushes.count() - 1) {
            // This is the last one
            break;
        }

        if (scale > d->scaledBrushes[current + 1].scale()) {
            // We fit in between the two.
            *belowBrush = &(d->scaledBrushes[current + 1]);
            break;
        }

        current++;
    }
}

KisQImagemaskSP KisBrush::scaleSinglePixelMask(double scale, quint8 maskValue, double subPixelX, double subPixelY)
{
    int srcWidth = 1;
    int srcHeight = 1;
    int dstWidth = 2;
    int dstHeight = 2;
    KisQImagemaskSP outputMask = KisQImagemaskSP(new KisQImagemask(dstWidth, dstHeight));
    Q_CHECK_PTR(outputMask);

    double a = subPixelX;
    double b = subPixelY;

    for (int y = 0; y < dstHeight; y++) {
        for (int x = 0; x < dstWidth; x++) {

            quint8 topLeft = (x > 0 && y > 0) ? maskValue : OPACITY_TRANSPARENT;
            quint8 bottomLeft = (x > 0 && y < srcHeight) ? maskValue : OPACITY_TRANSPARENT;
            quint8 topRight = (x < srcWidth && y > 0) ? maskValue : OPACITY_TRANSPARENT;
            quint8 bottomRight = (x < srcWidth && y < srcHeight) ? maskValue : OPACITY_TRANSPARENT;

            // Bi-linear interpolation
            int d = static_cast<int>(a * b * topLeft
                                     + a * (1 - b) * bottomLeft
                                     + (1 - a) * b * topRight
                                     + (1 - a) * (1 - b) * bottomRight + 0.5);

            // Multiply by the square of the scale because a 0.5x0.5 pixel
            // has 0.25 the value of the 1x1.
            d = static_cast<int>(d * scale * scale + 0.5);

            if (d < OPACITY_TRANSPARENT) {
                d = OPACITY_TRANSPARENT;
            } else if (d > OPACITY_OPAQUE) {
                d = OPACITY_OPAQUE;
            }

            outputMask->setAlphaAt(x, y, static_cast<quint8>(d));
        }
    }

    return outputMask;
}

QImage KisBrush::scaleSinglePixelImage(double scale, QRgb pixel, double subPixelX, double subPixelY)
{
    int srcWidth = 1;
    int srcHeight = 1;
    int dstWidth = 2;
    int dstHeight = 2;

    QImage outputImage(dstWidth, dstHeight, QImage::Format_ARGB32);

    double a = subPixelX;
    double b = subPixelY;

    for (int y = 0; y < dstHeight; y++) {
        for (int x = 0; x < dstWidth; x++) {

            QRgb topLeft = (x > 0 && y > 0) ? pixel : qRgba(0, 0, 0, 0);
            QRgb bottomLeft = (x > 0 && y < srcHeight) ? pixel : qRgba(0, 0, 0, 0);
            QRgb topRight = (x < srcWidth && y > 0) ? pixel : qRgba(0, 0, 0, 0);
            QRgb bottomRight = (x < srcWidth && y < srcHeight) ? pixel : qRgba(0, 0, 0, 0);

            // Bi-linear interpolation. Images are in pre-multiplied form.
            int red = static_cast<int>(a * b * qRed(topLeft)
                                       + a * (1 - b) * qRed(bottomLeft)
                                       + (1 - a) * b * qRed(topRight)
                                       + (1 - a) * (1 - b) * qRed(bottomRight) + 0.5);
            int green = static_cast<int>(a * b * qGreen(topLeft)
                                         + a * (1 - b) * qGreen(bottomLeft)
                                         + (1 - a) * b * qGreen(topRight)
                                         + (1 - a) * (1 - b) * qGreen(bottomRight) + 0.5);
            int blue = static_cast<int>(a * b * qBlue(topLeft)
                                        + a * (1 - b) * qBlue(bottomLeft)
                                        + (1 - a) * b * qBlue(topRight)
                                        + (1 - a) * (1 - b) * qBlue(bottomRight) + 0.5);
            int alpha = static_cast<int>(a * b * qAlpha(topLeft)
                                         + a * (1 - b) * qAlpha(bottomLeft)
                                         + (1 - a) * b * qAlpha(topRight)
                                         + (1 - a) * (1 - b) * qAlpha(bottomRight) + 0.5);

            // Multiply by the square of the scale because a 0.5x0.5 pixel
            // has 0.25 the value of the 1x1.
            alpha = static_cast<int>(alpha * scale * scale + 0.5);

            // Apply to the color channels too since we are
            // storing pre-multiplied by alpha.
            red = static_cast<int>(red * scale * scale + 0.5);
            green = static_cast<int>(green * scale * scale + 0.5);
            blue = static_cast<int>(blue * scale * scale + 0.5);

            if (red < 0) {
                red = 0;
            } else if (red > 255) {
                red = 255;
            }

            if (green < 0) {
                green = 0;
            } else if (green > 255) {
                green = 255;
            }

            if (blue < 0) {
                blue = 0;
            } else if (blue > 255) {
                blue = 255;
            }

            if (alpha < 0) {
                alpha = 0;
            } else if (alpha > 255) {
                alpha = 255;
            }

            outputImage.setPixel(x, y, qRgba(red, green, blue, alpha));
        }
    }

    return outputImage;
}

QImage KisBrush::interpolate(const QImage& image1, const QImage& image2, double t)
{
    Q_ASSERT((image1.width() == image2.width()) && (image1.height() == image2.height()));

    int width = image1.width();
    int height = image1.height();

    QImage outputImage(width, height, QImage::Format_ARGB32);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            QRgb image1pixel = image1.pixel(x, y);
            QRgb image2pixel = image2.pixel(x, y);

            // Images are in pre-multiplied alpha format.
            int red = static_cast<int>((1 - t) * qRed(image1pixel) + t * qRed(image2pixel) + 0.5);
            int green = static_cast<int>((1 - t) * qGreen(image1pixel) + t * qGreen(image2pixel) + 0.5);
            int blue = static_cast<int>((1 - t) * qBlue(image1pixel) + t * qBlue(image2pixel) + 0.5);
            int alpha = static_cast<int>((1 - t) * qAlpha(image1pixel) + t * qAlpha(image2pixel) + 0.5);

            if (red < 0) {
                red = 0;
            } else if (red > 255) {
                red = 255;
            }

            if (green < 0) {
                green = 0;
            } else if (green > 255) {
                green = 255;
            }

            if (blue < 0) {
                blue = 0;
            } else if (blue > 255) {
                blue = 255;
            }

            if (alpha < 0) {
                alpha = 0;
            } else if (alpha > 255) {
                alpha = 255;
            }

            outputImage.setPixel(x, y, qRgba(red, green, blue, alpha));
        }
    }

    return outputImage;
}

void KisBrush::resetBoundary()
{
    delete d->boundary;
    d->boundary = 0;
}

void KisBrush::generateBoundary() const
{
    KisFixedPaintDeviceSP dev;
    int w = width();
    int h = height();

    if (brushType() == IMAGE || brushType() == PIPE_IMAGE) {
        dev = image(KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0), 1.0, 0.0, KisPaintInformation());
    } else {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        dev = new KisFixedPaintDevice(cs);
        mask(dev, KoColor(Qt::black, cs) , 1.0, 1.0, 0.0, KisPaintInformation());
#if 0
        KisQImagemaskSP amask = mask(KisPaintInformation());
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
        dev = new KisPaintDevice(cs, "tmp for generateBoundary");

        KisHLineIteratorPixel it = dev->createHLineIterator(0, 0, w);

        for (int y = 0; y < h; y++) {
            int x = 0;

            while (!it.isDone()) {
                cs->setAlpha(it.rawData(), amask->alphaAt(x++, y), 1);
                ++it;
            }
            it.nextRow();
        }
#endif
    }

    d->boundary = new KisBoundary(dev.data());
    d->boundary->generateBoundary(w, h);
}

const KisBoundary* KisBrush::boundary() const
{
    if (!d->boundary)
        generateBoundary();
    return d->boundary;
}

