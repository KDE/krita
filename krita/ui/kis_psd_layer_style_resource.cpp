/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_psd_layer_style_resource.h"

#include <QBuffer>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QCryptographicHash>

#include <kdebug.h>
#include <klocale.h>

#include "kis_psd_layer_style.h"

KisPSDLayerStyleCollectionResource::KisPSDLayerStyleCollectionResource(const QString &filename)
    : KoResource(filename)
{

}

KisPSDLayerStyleCollectionResource::~KisPSDLayerStyleCollectionResource()
{
    m_layerStyles.clear();
}

bool KisPSDLayerStyleCollectionResource::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;

    bool result;
    if (!file.open(QIODevice::ReadOnly)) {
        kWarning() << "Can't open file " << filename();
        return false;
    }
    result = loadFromDevice(&file);
    file.close();

    return result;
}

bool KisPSDLayerStyleCollectionResource::loadFromDevice(QIODevice *dev)
{
    m_layerStyles = KisPSDLayerStyle::readASL(dev);
    return true;
}

bool KisPSDLayerStyleCollectionResource::save()
{
    QFile file(filename());
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    bool res = saveToDevice(&file);
    file.close();
    return res;
}

bool KisPSDLayerStyleCollectionResource::saveToDevice(QIODevice *dev) const
{
    return KisPSDLayerStyle::writeASL(dev, m_layerStyles);
}

QString KisPSDLayerStyleCollectionResource::defaultFileExtension() const
{
    return QString(".asl");
}

KisPSDLayerStyleCollectionResource::StylesVector
KisPSDLayerStyleCollectionResource::layerStyles() const
{
    return m_layerStyles;
}

QByteArray KisPSDLayerStyleCollectionResource::generateMD5() const
{
    if (m_layerStyles.size() > 0) {
        QByteArray ba;
        QBuffer buf(&ba);
        saveToDevice(&buf);
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(ba);
        return md5.result();
    }
    return QByteArray();
}
