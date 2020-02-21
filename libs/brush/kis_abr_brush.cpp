/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2007 Eric Lamarque <eric.lamarque@free.fr>
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
#include "kis_abr_brush.h"
#include "kis_abr_brush_collection.h"

#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPoint>
#include <QByteArray>
#include <QBuffer>
#include <QCryptographicHash>

#include <klocalizedstring.h>

#include <KoColor.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_image.h"

#define DEFAULT_SPACING 0.25

KisAbrBrush::KisAbrBrush(const QString& filename, KisAbrBrushCollection *parent)
    : KoEphemeralResource<KisScalingSizeBrush>(filename)
    , m_parent(parent)
{
    setBrushType(INVALID);
    setHasColor(false);
    setSpacing(DEFAULT_SPACING);
}

KisAbrBrush::KisAbrBrush(const KisAbrBrush& rhs)
    : KoEphemeralResource<KisScalingSizeBrush>(rhs)
    , m_parent(0)
{
    // Warning! The brush became detached from the parent collection!
}

KisAbrBrush::KisAbrBrush(const KisAbrBrush& rhs, KisAbrBrushCollection *parent)
    : KoEphemeralResource<KisScalingSizeBrush>(rhs)
    , m_parent(parent)
{
}

KisAbrBrush &KisAbrBrush::operator=(const KisAbrBrush &rhs)
{
    if (*this != rhs) {
        m_parent = rhs.m_parent; // XXX: should this also be zero as in the first copy constructor?
    }
    return *this;
}

KoResourceSP KisAbrBrush::clone() const
{
    return KoResourceSP(new KisAbrBrush(*this));
}

void KisAbrBrush::setBrushTipImage(const QImage& image)
{
    setValid(true);
    setBrushType(MASK);
    setHasColor(false);

    KisBrush::setBrushTipImage(image);
}

void KisAbrBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    e.setAttribute("name", name()); // legacy
    predefinedBrushToXML("abr_brush", e);
    KisBrush::toXML(d, e);
}

QString KisAbrBrush::defaultFileExtension() const
{
    return QString();
}

QImage KisAbrBrush::brushTipImage() const
{
    if (KisBrush::brushTipImage().isNull() && m_parent) {
        m_parent->load();
    }
    return KisBrush::brushTipImage();
}
