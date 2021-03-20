/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2007 Eric Lamarque <eric.lamarque@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

KoResourceSP KisAbrBrush::clone() const
{
    return KoResourceSP(new KisAbrBrush(*this));
}

void KisAbrBrush::setBrushTipImage(const QImage& image)
{
    setValid(true);
    setBrushType(MASK);

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
