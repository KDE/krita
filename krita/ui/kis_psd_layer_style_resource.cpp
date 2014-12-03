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

KisPSDLayerStyleResource::KisPSDLayerStyleResource(const QString &filename)
    : KoResource(filename)
    , m_layerStyle(0)
{

}

KisPSDLayerStyleResource::~KisPSDLayerStyleResource()
{
    delete m_layerStyle;
}

bool KisPSDLayerStyleResource::load()
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

bool KisPSDLayerStyleResource::loadFromDevice(QIODevice *dev)
{
    if (!m_layerStyle) {
        m_layerStyle = new KisPSDLayerStyle();
    }
    return m_layerStyle->readASL(dev);
}

bool KisPSDLayerStyleResource::save()
{
    QFile file(filename());
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    bool res = saveToDevice(&file);
    file.close();
    return res;
}

bool KisPSDLayerStyleResource::saveToDevice(QIODevice *dev) const
{
    return m_layerStyle->writeASL(dev);
}

QString KisPSDLayerStyleResource::defaultFileExtension() const
{
    return QString(".asl");
}

QByteArray KisPSDLayerStyleResource::generateMD5() const
{
    if (m_layerStyle) {
        QByteArray ba;
        QBuffer buf(&ba);
        saveToDevice(&buf);
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(ba);
        return md5.result();
    }
    return QByteArray();
}
