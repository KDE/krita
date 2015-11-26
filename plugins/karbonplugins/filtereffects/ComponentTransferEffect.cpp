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

#include "ComponentTransferEffect.h"
#include "ColorChannelConversion.h"
#include <KoFilterEffectRenderContext.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <klocalizedstring.h>
#include <QRect>
#include <QImage>
#include <math.h>

ComponentTransferEffect::ComponentTransferEffect()
    : KoFilterEffect(ComponentTransferEffectId, i18n("Component transfer"))
{
}

ComponentTransferEffect::Function ComponentTransferEffect::function(Channel channel) const
{
    return m_data[channel].function;
}

void ComponentTransferEffect::setFunction(Channel channel, Function function)
{
    m_data[channel].function = function;
}

QList<qreal> ComponentTransferEffect::tableValues(Channel channel) const
{
    return m_data[channel].tableValues;
}

void ComponentTransferEffect::setTableValues(Channel channel, QList<qreal> tableValues)
{
    m_data[channel].tableValues = tableValues;
}

void ComponentTransferEffect::setSlope(Channel channel, qreal slope)
{
    m_data[channel].slope = slope;
}

qreal ComponentTransferEffect::slope(Channel channel) const
{
    return m_data[channel].slope;
}

void ComponentTransferEffect::setIntercept(Channel channel, qreal intercept)
{
    m_data[channel].intercept = intercept;
}

qreal ComponentTransferEffect::intercept(Channel channel) const
{
    return m_data[channel].intercept;
}

void ComponentTransferEffect::setAmplitude(Channel channel, qreal amplitude)
{
    m_data[channel].amplitude = amplitude;
}

qreal ComponentTransferEffect::amplitude(Channel channel) const
{
    return m_data[channel].amplitude;
}

void ComponentTransferEffect::setExponent(Channel channel, qreal exponent)
{
    m_data[channel].exponent = exponent;
}

qreal ComponentTransferEffect::exponent(Channel channel) const
{
    return m_data[channel].exponent;
}

void ComponentTransferEffect::setOffset(Channel channel, qreal offset)
{
    m_data[channel].offset = offset;
}

qreal ComponentTransferEffect::offset(Channel channel) const
{
    return m_data[channel].offset;
}

QImage ComponentTransferEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &context) const
{
    QImage result = image;

    const QRgb *src = (const QRgb *)image.constBits();
    QRgb *dst = (QRgb *)result.bits();
    int w = result.width();

    qreal sa, sr, sg, sb;
    qreal da, dr, dg, db;
    int pixel;

    const QRect roi = context.filterRegion().toRect();
    const int minRow = roi.top();
    const int maxRow = roi.bottom();
    const int minCol = roi.left();
    const int maxCol = roi.right();

    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            pixel = row * w + col;
            const QRgb &s = src[pixel];

            sa = fromIntColor[qAlpha(s)];
            sr = fromIntColor[qRed(s)];
            sg = fromIntColor[qGreen(s)];
            sb = fromIntColor[qBlue(s)];
            // the matrix is applied to non-premultiplied color values
            // so we have to convert colors by dividing by alpha value
            if (sa > 0.0 && sa < 1.0) {
                sr /= sa;
                sb /= sa;
                sg /= sa;
            }

            dr = transferChannel(ChannelR, sr);
            dg = transferChannel(ChannelG, sg);
            db = transferChannel(ChannelB, sb);
            da = transferChannel(ChannelA, sa);

            da *= 255.0;

            // set pre-multiplied color values on destination image
            dst[pixel] = qRgba(static_cast<quint8>(qBound(qreal(0.0), dr * da, qreal(255.0))),
                               static_cast<quint8>(qBound(qreal(0.0), dg * da, qreal(255.0))),
                               static_cast<quint8>(qBound(qreal(0.0), db * da, qreal(255.0))),
                               static_cast<quint8>(qBound(qreal(0.0), da, qreal(255.0))));
        }
    }

    return result;
}

qreal ComponentTransferEffect::transferChannel(Channel channel, qreal value) const
{
    const Data &d = m_data[channel];

    switch (d.function) {
    case Identity:
        return value;
    case Table: {
        qreal valueCount = d.tableValues.count() - 1;
        if (valueCount < 0.0) {
            return value;
        }
        qreal k1 = static_cast<int>(value * valueCount);
        qreal k2 = qMin(k1 + 1, valueCount);
        qreal vk1 = d.tableValues[k1];
        qreal vk2 = d.tableValues[k2];
        return vk1 + (value - static_cast<qreal>(k1) / valueCount) * valueCount * (vk2 - vk1);
    }
    case Discrete: {
        qreal valueCount = d.tableValues.count() - 1;
        if (valueCount < 0.0) {
            return value;
        }
        return d.tableValues[static_cast<int>(value * valueCount)];
    }
    case Linear:
        return d.slope * value + d.intercept;
    case Gamma:
        return d.amplitude * pow(value, d.exponent) + d.offset;
    }

    return value;
}

bool ComponentTransferEffect::load(const KoXmlElement &element, const KoFilterEffectLoadingContext &)
{
    if (element.tagName() != id()) {
        return false;
    }

    // reset data
    m_data[ChannelR] = Data();
    m_data[ChannelG] = Data();
    m_data[ChannelB] = Data();
    m_data[ChannelA] = Data();

    for (KoXmlNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        KoXmlElement node = n.toElement();
        if (node.tagName() == "feFuncR") {
            loadChannel(ChannelR, node);
        } else if (node.tagName() == "feFuncG") {
            loadChannel(ChannelG, node);
        } else if (node.tagName() == "feFuncB") {
            loadChannel(ChannelB, node);
        } else if (node.tagName() == "feFuncA") {
            loadChannel(ChannelA, node);
        }
    }

    return true;
}

void ComponentTransferEffect::loadChannel(Channel channel, const KoXmlElement &element)
{
    QString typeStr = element.attribute("type");
    if (typeStr.isEmpty()) {
        return;
    }

    Data &d = m_data[channel];

    if (typeStr == "table" || typeStr == "discrete") {
        d.function = typeStr == "table" ? Table : Discrete;
        QString valueStr = element.attribute("tableValues");
        QStringList values = valueStr.split(QRegExp("(\\s+|,)"), QString::SkipEmptyParts);
        Q_FOREACH (const QString &v, values) {
            d.tableValues.append(v.toDouble());
        }
    } else if (typeStr == "linear") {
        d.function = Linear;
        if (element.hasAttribute("slope")) {
            d.slope = element.attribute("slope").toDouble();
        }
        if (element.hasAttribute("intercept")) {
            d.intercept = element.attribute("intercept").toDouble();
        }
    } else if (typeStr == "gamma") {
        d.function = Gamma;
        if (element.hasAttribute("amplitude")) {
            d.amplitude = element.attribute("amplitude").toDouble();
        }
        if (element.hasAttribute("exponent")) {
            d.exponent = element.attribute("exponent").toDouble();
        }
        if (element.hasAttribute("offset")) {
            d.offset = element.attribute("offset").toDouble();
        }
    }
}

void ComponentTransferEffect::save(KoXmlWriter &writer)
{
    writer.startElement(ComponentTransferEffectId);

    saveCommonAttributes(writer);

    saveChannel(ChannelR, writer);
    saveChannel(ChannelG, writer);
    saveChannel(ChannelB, writer);
    saveChannel(ChannelA, writer);

    writer.endElement();
}

void ComponentTransferEffect::saveChannel(Channel channel, KoXmlWriter &writer)
{
    Function function = m_data[channel].function;
    // we can omit writing the transfer function when
    if (function == Identity) {
        return;
    }

    switch (channel) {
    case ChannelR:
        writer.startElement("feFuncR");
        break;
    case ChannelG:
        writer.startElement("feFuncG");
        break;
    case ChannelB:
        writer.startElement("feFuncB");
        break;
    case ChannelA:
        writer.startElement("feFuncA");
        break;
    }

    Data defaultData;
    const Data &currentData = m_data[channel];

    if (function == Linear) {
        writer.addAttribute("type", "linear");
        // only write non default data
        if (defaultData.slope != currentData.slope) {
            writer.addAttribute("slope", QString("%1").arg(currentData.slope));
        }
        if (defaultData.intercept != currentData.intercept) {
            writer.addAttribute("intercept", QString("%1").arg(currentData.intercept));
        }
    } else if (function == Gamma) {
        writer.addAttribute("type", "gamma");
        // only write non default data
        if (defaultData.amplitude != currentData.amplitude) {
            writer.addAttribute("amplitude", QString("%1").arg(currentData.amplitude));
        }
        if (defaultData.exponent != currentData.exponent) {
            writer.addAttribute("exponent", QString("%1").arg(currentData.exponent));
        }
        if (defaultData.offset != currentData.offset) {
            writer.addAttribute("offset", QString("%1").arg(currentData.offset));
        }
    } else {
        writer.addAttribute("type", function == Table ? "table" : "discrete");
        if (currentData.tableValues.count()) {
            QString tableStr;
            Q_FOREACH (qreal v, currentData.tableValues) {
                tableStr += QString("%1 ").arg(v);
            }
            writer.addAttribute("tableValues", tableStr.trimmed());
        }
    }

    writer.endElement();
}
