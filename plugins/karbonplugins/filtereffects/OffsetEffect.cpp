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

#include "OffsetEffect.h"
#include "KoFilterEffectRenderContext.h"
#include "KoFilterEffectLoadingContext.h"
#include "KoViewConverter.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include <klocalizedstring.h>
#include <QPainter>

OffsetEffect::OffsetEffect()
    : KoFilterEffect(OffsetEffectId, i18n("Offset"))
    , m_offset(0, 0)
{
}

QPointF OffsetEffect::offset() const
{
    return m_offset;
}

void OffsetEffect::setOffset(const QPointF &offset)
{
    m_offset = offset;
}

QImage OffsetEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &context) const
{
    if (m_offset.x() == 0.0 && m_offset.y() == 0.0) {
        return image;
    }

    // transform from bounding box coordinates
    QPointF offset = context.toUserSpace(m_offset);
    // transform to view coordinates
    offset = context.viewConverter()->documentToView(offset);

    QImage result(image.size(), image.format());
    result.fill(qRgba(0, 0, 0, 0));

    QPainter p(&result);
    p.drawImage(context.filterRegion().topLeft() + offset, image, context.filterRegion());
    return result;
}

bool OffsetEffect::load(const KoXmlElement &element, const KoFilterEffectLoadingContext &context)
{
    if (element.tagName() != id()) {
        return false;
    }

    if (element.hasAttribute("dx")) {
        m_offset.rx() = element.attribute("dx").toDouble();
    }
    if (element.hasAttribute("dy")) {
        m_offset.ry() = element.attribute("dy").toDouble();
    }

    m_offset = context.convertFilterPrimitiveUnits(m_offset);

    return true;
}

void OffsetEffect::save(KoXmlWriter &writer)
{
    writer.startElement(OffsetEffectId);

    saveCommonAttributes(writer);

    if (m_offset.x() != 0.0) {
        writer.addAttribute("dx", m_offset.x());
    }
    if (m_offset.y() != 0.0) {
        writer.addAttribute("dy", m_offset.x());
    }

    writer.endElement();
}
