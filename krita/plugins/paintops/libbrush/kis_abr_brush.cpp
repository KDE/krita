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
#include "kis_brush.h"

#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPoint>
#include <QByteArray>
#include <QBuffer>
#include <QCryptographicHash>

#include <kis_debug.h>
#include <klocale.h>

#include <KoColor.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_image.h"

#define DEFAULT_SPACING 0.25

KisAbrBrush::KisAbrBrush(const QString& filename, const QByteArray &parentMD5)
    : KisBrush(filename)
    , m_parentMD5(parentMD5)
{
    setBrushType(INVALID);
    setHasColor(false);
    setSpacing(DEFAULT_SPACING);
}

bool KisAbrBrush::load()
{
    return true;
}

bool KisAbrBrush::save()
{
    //Return true, otherwise the brush won't be added to the
    //resource server if the brush is loaded via import
    return true;
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

QByteArray KisAbrBrush::generateMD5() const
{
    if (!brushTipImage().isNull()) {
#if QT_VERSION >= 0x040700
        QByteArray ba = QByteArray::fromRawData((const char*)brushTipImage().constBits(), brushTipImage().byteCount());
#else
        QByteArray ba = QByteArray::fromRawData((const char*)brushTipImage().bits(), brushTipImage().byteCount());
#endif

        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(ba);
        md5.addData(m_parentMD5);

        return md5.result();
    }
    return QByteArray();

}
