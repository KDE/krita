/*
 *  kis_pattern.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_pattern.h"

#include <sys/types.h>
#include <netinet/in.h>

#include <limits.h>
#include <stdlib.h>

#include <QPoint>
#include <QSize>
#include <QImage>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

#include "KoColor.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

KisPattern::KisPattern(const QString& file)
    : KoPattern(file)
{
}

KisPattern::KisPattern(KisPaintDevice* image, int x, int y, int w, int h)
    : KoPattern("")
{
    // Forcefully convert to RGBA8
    // XXX profile and exposure?
    setImage(image->convertToQImage(0, x, y, w, h));
    setName(image->objectName());
}

KisPattern::~KisPattern()
{
}

KisPaintDeviceSP KisPattern::image(KoColorSpace * colorSpace) {
    // Check if there's already a pattern prepared for this colorspace
    QMap<QString, KisPaintDeviceSP>::const_iterator it = m_colorspaces.find(colorSpace->id());
    if (it != m_colorspaces.end())
        return (*it);

    // If not, create one
    KisPaintDeviceSP layer = KisPaintDeviceSP(new KisPaintDevice(colorSpace, "pattern"));

    Q_CHECK_PTR(layer);

    layer->convertFromQImage(img(),"");

    m_colorspaces[colorSpace->id()] = layer;
    return layer;
}

KisPattern* KisPattern::clone() const
{
    KisPattern* pattern = new KisPattern("");
    pattern->setImage(img());
    pattern->setName(name());
    return pattern;
}

#include "kis_pattern.moc"
