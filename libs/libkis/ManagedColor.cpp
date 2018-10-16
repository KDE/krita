/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ManagedColor.h"
#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorModelStandardIds.h>
#include <KoChannelInfo.h>

#include <QDomDocument>
#include <QDomElement>

#include <Canvas.h>

#include <kis_display_color_converter.h>
#include <KoColorDisplayRendererInterface.h>

struct ManagedColor::Private {
    KoColor color;
};

ManagedColor::ManagedColor(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    // Default black rgb color
}

ManagedColor::ManagedColor(const QString &colorModel, const QString &colorDepth, const QString &colorProfile, QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, colorProfile);
    if (colorSpace) {
        d->color = KoColor(colorSpace);
    }
}


ManagedColor::ManagedColor(KoColor color, QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    d->color = color;
}

ManagedColor::~ManagedColor()
{
}

bool ManagedColor::operator==(const ManagedColor &other) const
{
    return d->color == other.d->color;
}
QColor ManagedColor::colorForCanvas(Canvas *canvas) const
{
    QColor c = QColor(0,0,0);
    if (canvas && canvas->displayColorConverter() && canvas->displayColorConverter()->displayRendererInterface()) {
        KoColorDisplayRendererInterface *converter = canvas->displayColorConverter()->displayRendererInterface();
        if (converter) {
            c = converter->toQColor(d->color);
        } else {
            c = KoDumbColorDisplayRenderer::instance()->toQColor(d->color);
        }
    } else {
        c = KoDumbColorDisplayRenderer::instance()->toQColor(d->color);
    }
    return c;
}

QString ManagedColor::colorDepth() const
{
    return d->color.colorSpace()->colorDepthId().id();
}

QString ManagedColor::colorModel() const
{
    return d->color.colorSpace()->colorModelId().id();
}

QString ManagedColor::colorProfile() const
{
    return d->color.colorSpace()->profile()->name();
}

bool ManagedColor::setColorProfile(const QString &colorProfile)
{
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(colorProfile);
    if (!profile) return false;
    d->color.setProfile(profile);
    return true;
}

bool ManagedColor::setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile)
{
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, colorProfile);
    if (!colorSpace) return false;
    d->color.convertTo(colorSpace);
    return true;
}

QVector<float> ManagedColor::components() const
{
    QVector<float> values(d->color.colorSpace()->channelCount());
    d->color.colorSpace()->normalisedChannelsValue(d->color.data(), values);
    return values;
}

QVector<float> ManagedColor::componentsOrdered() const
{
    QVector<float> valuesUnsorted = components();
    QVector<float> values(d->color.colorSpace()->channelCount());
    for (int i=0; i<values.size();i++) {
        int location = KoChannelInfo::displayPositionToChannelIndex(i, d->color.colorSpace()->channels());
        values[location] = valuesUnsorted[i];
    }
    return values;
}

void ManagedColor::setComponents(const QVector<float> &values)
{
    d->color.colorSpace()->fromNormalisedChannelsValue(d->color.data(), values);
}

QString ManagedColor::toXML() const
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Color");
    root.setAttribute("bitdepth", colorDepth());
    doc.appendChild(root);
    d->color.toXML(doc, root);
    return doc.toString();
}

void ManagedColor::fromXML(const QString &xml)
{
    QDomDocument doc;
    doc.setContent(xml);
    QDomElement e = doc.documentElement();
    QDomElement c = e.firstChildElement("Color");
    KoColor kc;
    if (!c.isNull()) {
        QString colorDepthId = c.attribute("bitdepth", Integer8BitsColorDepthID.id());
        d->color = KoColor::fromXML(c, colorDepthId);
    }

}

QString ManagedColor::toQString()
{
    return KoColor::toQString(d->color);
}

KoColor ManagedColor::color() const
{
    return d->color;
}
