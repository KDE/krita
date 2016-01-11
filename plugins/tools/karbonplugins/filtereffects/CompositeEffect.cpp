/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "CompositeEffect.h"
#include "ColorChannelConversion.h"
#include <KoFilterEffectRenderContext.h>
#include <KoViewConverter.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <klocalizedstring.h>
#include <QDebug>
#include <QRect>
#include <QPainter>

CompositeEffect::CompositeEffect()
    : KoFilterEffect(CompositeEffectId, i18n("Composite"))
    , m_operation(CompositeOver)
{
    setRequiredInputCount(2);
    setMaximalInputCount(2);
    memset(m_k, 0, 4 * sizeof(qreal));
}

CompositeEffect::Operation CompositeEffect::operation() const
{
    return m_operation;
}

void CompositeEffect::setOperation(Operation op)
{
    m_operation = op;
}

const qreal *CompositeEffect::arithmeticValues() const
{
    return m_k;
}

void CompositeEffect::setArithmeticValues(qreal *values)
{
    memcpy(m_k, values, 4 * sizeof(qreal));
}

QImage CompositeEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &) const
{
    return image;
}

QImage CompositeEffect::processImages(const QList<QImage> &images, const KoFilterEffectRenderContext &context) const
{
    int imageCount = images.count();
    if (!imageCount) {
        return QImage();
    }

    QImage result = images[0];
    if (images.count() != 2) {
        return result;
    }

    if (m_operation == Arithmetic) {
        const QRgb *src = (QRgb *)images[1].constBits();
        QRgb *dst = (QRgb *)result.bits();
        int w = result.width();

        qreal sa, sr, sg, sb;
        qreal da, dr, dg, db;
        int pixel = 0;

        // TODO: do we have to calculate with non-premuliplied colors here ???

        QRect roi = context.filterRegion().toRect();
        for (int row = roi.top(); row < roi.bottom(); ++row) {
            for (int col = roi.left(); col < roi.right(); ++col) {
                pixel = row * w + col;
                const QRgb &s = src[pixel];
                QRgb &d = dst[pixel];

                sa = fromIntColor[qAlpha(s)];
                sr = fromIntColor[qRed(s)];
                sg = fromIntColor[qGreen(s)];
                sb = fromIntColor[qBlue(s)];

                da = fromIntColor[qAlpha(d)];
                dr = fromIntColor[qRed(d)];
                dg = fromIntColor[qGreen(d)];
                db = fromIntColor[qBlue(d)];

                da = m_k[0] * sa * da + m_k[1] * da + m_k[2] * sa + m_k[3];
                dr = m_k[0] * sr * dr + m_k[1] * dr + m_k[2] * sr + m_k[3];
                dg = m_k[0] * sg * dg + m_k[1] * dg + m_k[2] * sg + m_k[3];
                db = m_k[0] * sb * db + m_k[1] * db + m_k[2] * sb + m_k[3];

                da *= 255.0;

                // set pre-multiplied color values on destination image
                d = qRgba(static_cast<quint8>(qBound(qreal(0.0), dr * da, qreal(255.0))),
                          static_cast<quint8>(qBound(qreal(0.0), dg * da, qreal(255.0))),
                          static_cast<quint8>(qBound(qreal(0.0), db * da, qreal(255.0))),
                          static_cast<quint8>(qBound(qreal(0.0), da, qreal(255.0))));
            }
        }
    } else {
        QPainter painter(&result);

        switch (m_operation) {
        case CompositeOver:
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            break;
        case CompositeIn:
            painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            break;
        case CompositeOut:
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            break;
        case CompositeAtop:
            painter.setCompositionMode(QPainter::CompositionMode_DestinationAtop);
            break;
        case CompositeXor:
            painter.setCompositionMode(QPainter::CompositionMode_Xor);
            break;
        default:
            // no composition mode
            break;
        }
        painter.drawImage(context.filterRegion(), images[1], context.filterRegion());
    }

    return result;
}

bool CompositeEffect::load(const KoXmlElement &element, const KoFilterEffectLoadingContext &)
{
    if (element.tagName() != id()) {
        return false;
    }

    QString opStr = element.attribute("operator");
    if (opStr == "over") {
        m_operation = CompositeOver;
    } else if (opStr == "in") {
        m_operation = CompositeIn;
    } else if (opStr == "out") {
        m_operation = CompositeOut;
    } else if (opStr == "atop") {
        m_operation = CompositeAtop;
    } else if (opStr == "xor") {
        m_operation = CompositeXor;
    } else if (opStr == "arithmetic") {
        m_operation = Arithmetic;
        if (element.hasAttribute("k1")) {
            m_k[0] = element.attribute("k1").toDouble();
        }
        if (element.hasAttribute("k2")) {
            m_k[1] = element.attribute("k2").toDouble();
        }
        if (element.hasAttribute("k3")) {
            m_k[2] = element.attribute("k3").toDouble();
        }
        if (element.hasAttribute("k4")) {
            m_k[3] = element.attribute("k4").toDouble();
        }
    } else {
        return false;
    }

    if (element.hasAttribute("in2")) {
        if (inputs().count() == 2) {
            setInput(1, element.attribute("in2"));
        } else {
            addInput(element.attribute("in2"));
        }
    }

    return true;
}

void CompositeEffect::save(KoXmlWriter &writer)
{
    writer.startElement(CompositeEffectId);

    saveCommonAttributes(writer);

    switch (m_operation) {
    case CompositeOver:
        writer.addAttribute("operator", "over");
        break;
    case CompositeIn:
        writer.addAttribute("operator", "in");
        break;
    case CompositeOut:
        writer.addAttribute("operator", "out");
        break;
    case CompositeAtop:
        writer.addAttribute("operator", "atop");
        break;
    case CompositeXor:
        writer.addAttribute("operator", "xor");
        break;
    case Arithmetic:
        writer.addAttribute("operator", "arithmetic");
        writer.addAttribute("k1", QString("%1").arg(m_k[0]));
        writer.addAttribute("k2", QString("%1").arg(m_k[1]));
        writer.addAttribute("k3", QString("%1").arg(m_k[2]));
        writer.addAttribute("k4", QString("%1").arg(m_k[3]));
        break;
    }

    writer.addAttribute("in2", inputs().at(1));

    writer.endElement();
}
