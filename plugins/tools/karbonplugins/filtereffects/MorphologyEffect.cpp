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

#include "MorphologyEffect.h"
#include "KoFilterEffectRenderContext.h"
#include "KoFilterEffectLoadingContext.h"
#include "KoViewConverter.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include <klocalizedstring.h>
#include <QRect>
#include <QImage>
#include <cmath>

MorphologyEffect::MorphologyEffect()
    : KoFilterEffect(MorphologyEffectId, i18n("Morphology"))
    , m_radius(0, 0)
    , m_operator(Erode)
{
}

QPointF MorphologyEffect::morphologyRadius() const
{
    return m_radius;
}

void MorphologyEffect::setMorphologyRadius(const QPointF &radius)
{
    m_radius = radius;
}

MorphologyEffect::Operator MorphologyEffect::morphologyOperator() const
{
    return m_operator;
}

void MorphologyEffect::setMorphologyOperator(Operator op)
{
    m_operator = op;
}

QImage MorphologyEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &context) const
{
    QImage result = image;

    QPointF radius = context.toUserSpace(m_radius);

    const int rx = static_cast<int>(ceil(radius.x()));
    const int ry = static_cast<int>(ceil(radius.y()));

    const int w = result.width();
    const int h = result.height();

    // setup mask
    const int maskSize = (1 + 2 * rx) * (1 + 2 * ry);
    int *mask = new int[maskSize];
    int index = 0;
    for (int y = -ry; y <= ry; ++y) {
        for (int x = -rx; x <= rx; ++x) {
            mask[index] = y * w + x;
            index++;
        }
    }

    int dstPixel, srcPixel;
    uchar s0, s1, s2, s3;
    const uchar *src = image.constBits();
    uchar *dst = result.bits();

    const QRect roi = context.filterRegion().toRect();
    const int minX = qMax(rx, roi.left());
    const int maxX = qMin(w - rx, roi.right());
    const int minY = qMax(ry, roi.top());
    const int maxY = qMin(h - ry, roi.bottom());
    const int defValue = m_operator == Erode ? 255 : 0;

    uchar *d = 0;

    for (int row = minY; row < maxY; ++row) {
        for (int col = minX; col < maxX; ++col) {
            dstPixel = row * w + col;
            s0 = s1 = s2 = s3 = defValue;
            for (int i = 0; i < maskSize; ++i) {
                srcPixel = dstPixel + mask[i];
                const uchar *s = &src[4 * srcPixel];
                if (m_operator == Erode) {
                    s0 = qMin(s0, s[0]);
                    s1 = qMin(s1, s[1]);
                    s2 = qMin(s2, s[2]);
                    s3 = qMin(s3, s[3]);
                } else {
                    s0 = qMax(s0, s[0]);
                    s1 = qMax(s1, s[1]);
                    s2 = qMax(s2, s[2]);
                    s3 = qMax(s3, s[3]);
                }
            }
            d = &dst[4 * dstPixel];
            d[0] = s0;
            d[1] = s1;
            d[2] = s2;
            d[3] = s3;
        }
    }

    delete [] mask;

    return result;
}

bool MorphologyEffect::load(const KoXmlElement &element, const KoFilterEffectLoadingContext &context)
{
    if (element.tagName() != id()) {
        return false;
    }

    m_radius = QPointF();
    m_operator = Erode;

    if (element.hasAttribute("radius")) {
        QString radiusStr = element.attribute("radius").trimmed();
        QStringList params = radiusStr.replace(',', ' ').simplified().split(' ');
        switch (params.count()) {
        case 1:
            m_radius.rx() = params[0].toDouble() * 72. / 90.;
            m_radius.ry() = m_radius.x();
            break;
        case 2:
            m_radius.rx() = params[0].toDouble() * 72. / 90.;
            m_radius.ry() = params[1].toDouble() * 72. / 90.;
            break;
        default:
            m_radius = QPointF();
        }
    }

    m_radius = context.convertFilterPrimitiveUnits(m_radius);

    if (element.hasAttribute("operator")) {
        QString op = element.attribute("operator");
        if (op == "dilate") {
            m_operator = Dilate;
        }
    }

    return true;
}

void MorphologyEffect::save(KoXmlWriter &writer)
{
    writer.startElement(MorphologyEffectId);

    saveCommonAttributes(writer);

    if (m_operator != Erode) {
        writer.addAttribute("operator", "dilate");
    }
    if (!m_radius.isNull()) {
        if (m_radius.x() == m_radius.y()) {
            writer.addAttribute("radius", QString("%1").arg(m_radius.x()));
        } else {
            writer.addAttribute("radius", QString("%1 %2").arg(m_radius.x()).arg(m_radius.y()));
        }
    }

    writer.endElement();
}
