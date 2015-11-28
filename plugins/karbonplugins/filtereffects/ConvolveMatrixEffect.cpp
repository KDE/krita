/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ConvolveMatrixEffect.h"
#include "KoFilterEffectRenderContext.h"
#include "KoFilterEffectLoadingContext.h"
#include "KoViewConverter.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include <klocalizedstring.h>
#include <QRect>
#include <QVector>
#include <QImage>
#include <QColor>

#include <cmath>

ConvolveMatrixEffect::ConvolveMatrixEffect()
    : KoFilterEffect(ConvolveMatrixEffectId, i18n("Convolve Matrix"))
{
    setDefaults();
}

void ConvolveMatrixEffect::setDefaults()
{
    m_order = QPoint(3, 3);
    m_divisor = 0.0;
    m_bias = 0.0;
    m_target = QPoint(-1, -1);
    m_edgeMode = Duplicate;
    m_preserveAlpha = false;
    m_kernel.resize(m_order.x()*m_order.y());
    for (int i = 0; i < m_kernel.size(); ++i) {
        m_kernel[i] = 0.0;
    }
    m_kernelUnitLength = QPointF(1, 1);
}

QPoint ConvolveMatrixEffect::order() const
{
    return m_order;
}

void ConvolveMatrixEffect::setOrder(const QPoint &order)
{
    m_order = QPoint(qMax(1, order.x()), qMax(1, order.y()));
}

QVector<qreal> ConvolveMatrixEffect::kernel() const
{
    return m_kernel;
}

void ConvolveMatrixEffect::setKernel(const QVector<qreal> &kernel)
{
    if (m_order.x()*m_order.y() != kernel.count()) {
        return;
    }
    m_kernel = kernel;
}

qreal ConvolveMatrixEffect::divisor() const
{
    return m_divisor;
}

void ConvolveMatrixEffect::setDivisor(qreal divisor)
{
    m_divisor = divisor;
}

qreal ConvolveMatrixEffect::bias() const
{
    return m_bias;
}

void ConvolveMatrixEffect::setBias(qreal bias)
{
    m_bias = bias;
}

QPoint ConvolveMatrixEffect::target() const
{
    return m_target;
}

void ConvolveMatrixEffect::setTarget(const QPoint &target)
{
    m_target = target;
}

ConvolveMatrixEffect::EdgeMode ConvolveMatrixEffect::edgeMode() const
{
    return m_edgeMode;
}

void ConvolveMatrixEffect::setEdgeMode(EdgeMode edgeMode)
{
    m_edgeMode = edgeMode;
}

bool ConvolveMatrixEffect::isPreserveAlphaEnabled() const
{
    return m_preserveAlpha;
}

void ConvolveMatrixEffect::enablePreserveAlpha(bool on)
{
    m_preserveAlpha = on;
}

QImage ConvolveMatrixEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &context) const
{
    QImage result = image;

    const int rx = m_order.x();
    const int ry = m_order.y();
    if (rx == 0 && ry == 0) {
        return result;
    }

    const int tx = m_target.x() >= 0 && m_target.x() <= rx ? m_target.x() : rx >> 1;
    const int ty = m_target.y() >= 0 && m_target.y() <= ry ? m_target.y() : ry >> 1;

    const int w = result.width();
    const int h = result.height();

    // setup mask
    const int maskSize = rx * ry;
    QVector<QPoint> offset(maskSize);
    int index = 0;
    for (int y = 0; y < ry; ++y) {
        for (int x = 0; x < rx; ++x) {
            offset[index] = QPoint(x - tx, y - ty);
            index++;
        }
    }

    qreal divisor = m_divisor;
    // if no divisor given, it is the sum of all kernel values
    // if sum of kernel values is zero, divisor is set to 1
    if (divisor == 0.0) {
        Q_FOREACH (qreal k, m_kernel) {
            divisor += k;
        }
        if (divisor == 0.0) {
            divisor = 1.0;
        }
    }

    int dstPixel, srcPixel;
    qreal sumA, sumR, sumG, sumB;
    const QRgb *src = (const QRgb *)image.constBits();
    QRgb *dst = (QRgb *)result.bits();

    const QRect roi = context.filterRegion().toRect();
    const int minX = roi.left();
    const int maxX = roi.right();
    const int minY = roi.top();
    const int maxY = roi.bottom();

    int srcRow, srcCol;
    for (int row = minY; row <= maxY; ++row) {
        for (int col = minX; col <= maxX; ++col) {
            dstPixel = row * w + col;
            sumA = sumR = sumG = sumB = 0;
            for (int i = 0; i < maskSize; ++i) {
                srcRow = row + offset[i].y();
                srcCol = col + offset[i].x();
                // handle top and bottom edge
                if (srcRow < 0 || srcRow >= h) {
                    switch (m_edgeMode) {
                    case Duplicate:
                        srcRow = srcRow >= h ? h - 1 : 0;
                        break;
                    case Wrap:
                        srcRow = (srcRow + h) % h;
                        break;
                    case None:
                        // zero for all color channels
                        continue;
                        break;
                    }
                }
                // handle left and right edge
                if (srcCol < 0 || srcCol >= w) {
                    switch (m_edgeMode) {
                    case Duplicate:
                        srcCol = srcCol >= w ? w - 1 : 0;
                        break;
                    case Wrap:
                        srcCol = (srcCol + w) % w;
                        break;
                    case None:
                        // zero for all color channels
                        continue;
                        break;
                    }
                }
                srcPixel = srcRow * w + srcCol;
                const QRgb &s = src[srcPixel];
                const qreal &k = m_kernel[i];
                if (!m_preserveAlpha) {
                    sumA += qAlpha(s) * k;
                }
                sumR += qRed(s) * k;
                sumG += qGreen(s) * k;
                sumB += qBlue(s) * k;
            }
            if (m_preserveAlpha) {
                dst[dstPixel] = qRgba(qBound(0, static_cast<int>(sumR / divisor + m_bias), 255),
                                      qBound(0, static_cast<int>(sumG / divisor + m_bias), 255),
                                      qBound(0, static_cast<int>(sumB / divisor + m_bias), 255),
                                      qAlpha(dst[dstPixel]));
            } else {
                dst[dstPixel] = qRgba(qBound(0, static_cast<int>(sumR / divisor + m_bias), 255),
                                      qBound(0, static_cast<int>(sumG / divisor + m_bias), 255),
                                      qBound(0, static_cast<int>(sumB / divisor + m_bias), 255),
                                      qBound(0, static_cast<int>(sumA / divisor + m_bias), 255));
            }
        }
    }

    return result;
}

bool ConvolveMatrixEffect::load(const KoXmlElement &element, const KoFilterEffectLoadingContext &/*context*/)
{
    if (element.tagName() != id()) {
        return false;
    }

    setDefaults();

    if (element.hasAttribute("order")) {
        QString orderStr = element.attribute("order");
        QStringList params = orderStr.replace(',', ' ').simplified().split(' ');
        switch (params.count()) {
        case 1:
            m_order.rx() = qMax(1, params[0].toInt());
            m_order.ry() = m_order.x();
            break;
        case 2:
            m_order.rx() = qMax(1, params[0].toInt());
            m_order.ry() = qMax(1, params[1].toInt());
            break;
        }
    }
    if (element.hasAttribute("kernelMatrix")) {
        QString matrixStr = element.attribute("kernelMatrix");
        // values are separated by whitespace and/or comma
        QStringList values = matrixStr.replace(',', ' ').simplified().split(' ');
        if (values.count() == m_order.x()*m_order.y()) {
            m_kernel.resize(values.count());
            for (int i = 0; i < values.count(); ++i) {
                m_kernel[i] = values[i].toDouble();
            }
        } else {
            m_kernel.resize(m_order.x()*m_order.y());
            for (int i = 0; i < m_kernel.size(); ++i) {
                m_kernel[i] = 0.0;
            }
        }
    }
    if (element.hasAttribute("divisor")) {
        m_divisor = element.attribute("divisor").toDouble();
    }
    if (element.hasAttribute("bias")) {
        m_bias = element.attribute("bias").toDouble();
    }
    if (element.hasAttribute("targetX")) {
        m_target.rx() = qBound<int>(0, element.attribute("targetX").toInt(), m_order.x());
    }
    if (element.hasAttribute("targetY")) {
        m_target.ry() = qBound<int>(0, element.attribute("targetY").toInt(), m_order.y());
    }
    if (element.hasAttribute("edgeMode")) {
        QString mode = element.attribute("edgeMode");
        if (mode == "wrap") {
            m_edgeMode = Wrap;
        } else if (mode == "none") {
            m_edgeMode = None;
        } else {
            m_edgeMode = Duplicate;
        }
    }
    if (element.hasAttribute("kernelUnitLength")) {
        QString kernelUnitLengthStr = element.attribute("kernelUnitLength");
        QStringList params = kernelUnitLengthStr.replace(',', ' ').simplified().split(' ');
        switch (params.count()) {
        case 1:
            m_kernelUnitLength.rx() = params[0].toDouble();
            m_kernelUnitLength.ry() = m_kernelUnitLength.x();
            break;
        case 2:
            m_kernelUnitLength.rx() = params[0].toDouble();
            m_kernelUnitLength.ry() = params[1].toDouble();
            break;
        }
    }
    if (element.hasAttribute("preserveAlpha")) {
        m_preserveAlpha = (element.attribute("preserveAlpha") == "true");
    }

    return true;
}

void ConvolveMatrixEffect::save(KoXmlWriter &writer)
{
    writer.startElement(ConvolveMatrixEffectId);

    saveCommonAttributes(writer);

    if (m_order.x() == m_order.y()) {
        writer.addAttribute("order", QString("%1").arg(m_order.x()));
    } else {
        writer.addAttribute("order", QString("%1 %2").arg(m_order.x()).arg(m_order.y()));
    }
    QString kernel;
    for (int i = 0; i < m_kernel.size(); ++i) {
        kernel += QString("%1 ").arg(m_kernel[i]);
    }
    writer.addAttribute("kernelMatrix", kernel);
    writer.addAttribute("divisor", QString("%1").arg(m_divisor));
    if (m_bias != 0.0) {
        writer.addAttribute("bias", QString("%1").arg(m_bias));
    }
    writer.addAttribute("targetX", QString("%1").arg(m_target.x()));
    writer.addAttribute("targetY", QString("%1").arg(m_target.y()));
    switch (m_edgeMode) {
    case Wrap:
        writer.addAttribute("edgeMode", "wrap");
        break;
    case None:
        writer.addAttribute("edgeMode", "none");
        break;
    case Duplicate:
        // fall through as it is the default
        break;
    }
    writer.addAttribute("kernelUnitLength", QString("%1 %2").arg(m_kernelUnitLength.x()).arg(m_kernelUnitLength.y()));
    if (m_preserveAlpha) {
        writer.addAttribute("preserveAlpha", "true");
    }

    writer.endElement();
}
