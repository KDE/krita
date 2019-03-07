/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "PresetImageProvider.h"

#include <KisResourceServerProvider.h>
#include <brushengine/kis_paintop_preset.h>

class PresetImageProvider::Private {
public:
    Private()
    {
        rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    }

    KisPaintOpPresetResourceServer * rserver ;
};

PresetImageProvider::PresetImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , d(new Private)
{
}

QImage PresetImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);
    QImage image(requestedSize, QImage::Format_ARGB32);
    QList<KisPaintOpPresetSP> resources = d->rserver->resources();
    int theID = id.toInt();
    if (theID >= 0 && theID < resources.count())
    {
        image = resources.at(theID)->image();
    }
    return image;
}
