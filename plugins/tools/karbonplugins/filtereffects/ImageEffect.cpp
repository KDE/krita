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

#include "ImageEffect.h"

#include "KoFilterEffectRenderContext.h"
#include "KoFilterEffectLoadingContext.h"
#include "KoViewConverter.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QBuffer>
#include <QPainter>
#include <QDebug>

#include <klocalizedstring.h>

ImageEffect::ImageEffect()
    : KoFilterEffect(ImageEffectId, i18n("Image"))
{
    setRequiredInputCount(0);
    setMaximalInputCount(0);
}

QImage ImageEffect::image() const
{
    return m_image;
}

void ImageEffect::setImage(const QImage &image)
{
    m_image = image;
}

QImage ImageEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &context) const
{
    QImage result(image.size(), QImage::Format_ARGB32_Premultiplied);
    result.fill(qRgba(0, 0, 0, 0));

    QPainter p(&result);
    p.drawImage(context.filterRegion(), m_image);
    return result;
}

bool ImageEffect::load(const KoXmlElement &element, const KoFilterEffectLoadingContext &context)
{
    if (element.tagName() != id()) {
        return false;
    }

    QString href = element.attribute("xlink:href");
    if (href.startsWith(QLatin1String("data:"))) {
        int start = href.indexOf("base64,");
        if (start <= 0 || !m_image.loadFromData(QByteArray::fromBase64(href.mid(start + 7).toLatin1()))) {
            return false;
        }
    } else if (!m_image.load(context.pathFromHref(href))) {
        return false;
    }

    return true;
}

void ImageEffect::save(KoXmlWriter &writer)
{
    writer.startElement(ImageEffectId);

    saveCommonAttributes(writer);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    if (m_image.save(&buffer, "PNG")) {
        QMimeDatabase db;
        const QString mimeType(db.mimeTypeForData(ba).name());
        writer.addAttribute("xlink:href", "data:" + mimeType + ";base64," + ba.toBase64());
    }

    writer.endElement();
}
