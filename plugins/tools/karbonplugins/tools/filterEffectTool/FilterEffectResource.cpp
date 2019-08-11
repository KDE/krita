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

#include "FilterEffectResource.h"
#include <KoFilterEffect.h>
#include <KoFilterEffectStack.h>
#include <KoFilterEffectRegistry.h>
#include <KoFilterEffectLoadingContext.h>
#include <KoXmlWriter.h>

#include <QDebug>

#include <QFile>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDomDocument>

double fromPercentage(QString s)
{
    if (s.endsWith('%')) {
        return s.remove('%').toDouble() / 100.0;
    } else {
        return s.toDouble();
    }
}

FilterEffectResource::FilterEffectResource(const QString &filename)
    : KoResource(filename)
{
}

bool FilterEffectResource::load()
{
    QFile file(filename());

    if (file.size() == 0) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    bool res = loadFromDevice(&file);

    file.close();
    return res;
}

bool FilterEffectResource::loadFromDevice(QIODevice *dev)
{
    if (!m_data.setContent(dev)) {
        return false;
    }

    setName(m_data.documentElement().attribute("id"));
    setValid(true);

    return true;
}

bool FilterEffectResource::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    bool result = saveToDevice(&file);
    file.close();
    return result;
}

bool FilterEffectResource::saveToDevice(QIODevice *dev) const
{
    m_data.documentElement().setAttribute("id", name());
    QByteArray ba = m_data.toByteArray(2);
    bool success = (dev->write(ba) == ba.size());
    return success;
}

QString FilterEffectResource::defaultFileExtension() const
{
    return QString(".svg");
}

FilterEffectResource *FilterEffectResource::fromFilterEffectStack(KoFilterEffectStack *filterStack)
{
    if (!filterStack) {
        return 0;
    }

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    KoXmlWriter writer(&buffer);

    filterStack->save(writer, "");

    buffer.close();

    FilterEffectResource *resource = new FilterEffectResource(QString());
    if (!resource->m_data.setContent(ba)) {
        delete resource;
        return 0;
    }

    return resource;
}

KoFilterEffectStack *FilterEffectResource::toFilterStack() const
{
    QScopedPointer<KoFilterEffectStack> filterStack(new KoFilterEffectStack());

    QByteArray data = m_data.toByteArray();
    KoXmlDocument doc;
    doc.setContent(data);
    KoXmlElement e = doc.documentElement();

    // only allow object bounding box units
    if (e.hasAttribute("filterUnits") && e.attribute("filterUnits") != "objectBoundingBox") {
        return 0;
    }

    if (e.attribute("primitiveUnits") != "objectBoundingBox") {
        return 0;
    }

    // parse filter region rectangle
    QRectF filterRegion;
    filterRegion.setX(fromPercentage(e.attribute("x", "-0.1")));
    filterRegion.setY(fromPercentage(e.attribute("y", "-0.1")));
    filterRegion.setWidth(fromPercentage(e.attribute("width", "1.2")));
    filterRegion.setHeight(fromPercentage(e.attribute("height", "1.2")));
    filterStack->setClipRect(filterRegion);

    KoFilterEffectLoadingContext context;

    KoFilterEffectRegistry *registry = KoFilterEffectRegistry::instance();

    // create the filter effects and add them to the shape
    for (KoXmlNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        KoXmlElement primitive = n.toElement();
        KoFilterEffect *filterEffect = registry->createFilterEffectFromXml(primitive, context);
        if (!filterEffect) {
            qWarning() << "filter effect" << primitive.tagName() << "is not implemented yet";
            continue;
        }

        // parse subregion
        qreal x = fromPercentage(primitive.attribute("x", "0"));
        qreal y = fromPercentage(primitive.attribute("y", "0"));
        qreal w = fromPercentage(primitive.attribute("width", "1"));
        qreal h = fromPercentage(primitive.attribute("height", "1"));
        QRectF subRegion(QPointF(x, y), QSizeF(w, h));

        if (primitive.hasAttribute("in")) {
            filterEffect->setInput(0, primitive.attribute("in"));
        }
        if (primitive.hasAttribute("result")) {
            filterEffect->setOutput(primitive.attribute("result"));
        }

        filterEffect->setFilterRect(subRegion);

        filterStack->appendFilterEffect(filterEffect);
    }

    return filterStack.take();
}

QByteArray FilterEffectResource::generateMD5() const
{
    QByteArray ba = m_data.toByteArray();
    if (!ba.isEmpty()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(ba);
        return md5.result();
    }
    return ba;

}
