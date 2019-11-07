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
#include <QFileInfo>
#include <QCryptographicHash>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include "kis_psd_layer_style.h"
#include "kis_asl_layer_style_serializer.h"

#include "kis_layer.h"


KisPSDLayerStyleCollectionResource::KisPSDLayerStyleCollectionResource(const QString &filename)
    : KoResource(filename)
{
    if (!filename.isEmpty()) {
        setName(QFileInfo(filename).fileName());
    }
}

KisPSDLayerStyleCollectionResource::~KisPSDLayerStyleCollectionResource()
{
    m_layerStyles.clear();
}

KoResourceSP KisPSDLayerStyleCollectionResource::clone() const
{
    Q_ASSERT(0);
    return 0;
}

bool KisPSDLayerStyleCollectionResource::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;

    bool result;
    if (!file.open(QIODevice::ReadOnly)) {
        dbgKrita << "Can't open file " << filename();
        return false;
    }
    result = loadFromDevice(&file);
    file.close();

    setName(QFileInfo(filename()).fileName());

    return result;
}

bool KisPSDLayerStyleCollectionResource::loadFromDevice(QIODevice *dev)
{
    KisAslLayerStyleSerializer serializer;
    serializer.readFromDevice(dev);
    m_layerStyles = serializer.styles();
    setValid(true);

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
    if (m_layerStyles.isEmpty()) return true;

    KisAslLayerStyleSerializer serializer;
    serializer.setStyles(m_layerStyles);
    serializer.saveToDevice(dev);

    return true;
}

QString KisPSDLayerStyleCollectionResource::defaultFileExtension() const
{
    return QString(".asl");
}

KisPSDLayerStyleCollectionResource::StylesVector KisPSDLayerStyleCollectionResource::layerStyles() const
{
    return m_layerStyles;
}

void KisPSDLayerStyleCollectionResource::setLayerStyles(StylesVector styles)
{
    m_layerStyles = styles;
    setValid(!m_layerStyles.isEmpty());
}

QByteArray KisPSDLayerStyleCollectionResource::generateMD5() const
{
    if (m_layerStyles.size() > 0) {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        saveToDevice(&buf);
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(buf.buffer());
        return md5.result();
    }
    return QByteArray();
}

void KisPSDLayerStyleCollectionResource::collectAllLayerStyles(KisNodeSP root)
{
    KisLayer* layer = qobject_cast<KisLayer*>(root.data());

    if (layer && layer->layerStyle()) {
        KisPSDLayerStyleSP clone = layer->layerStyle()->clone().dynamicCast<KisPSDLayerStyle>();
        clone->setName(i18nc("Auto-generated layer style name for embedded styles (style itself)", "<%1> (embedded)", layer->name()));
        m_layerStyles << clone;
        setValid(true);
    }

    KisNodeSP child = root->firstChild();
    while (child) {
        collectAllLayerStyles(child);
        child = child->nextSibling();
    }
}

void KisPSDLayerStyleCollectionResource::assignAllLayerStyles(KisNodeSP root)
{
    KisLayer* layer = qobject_cast<KisLayer*>(root.data());

    if (layer && layer->layerStyle()) {
        QUuid uuid = layer->layerStyle()->uuid();

        bool found = false;

        Q_FOREACH (KisPSDLayerStyleSP style, m_layerStyles) {
            if (style->uuid() == uuid) {
                layer->setLayerStyle(style->clone().dynamicCast<KisPSDLayerStyle>());
                found = true;
                break;
            }
        }

        if (!found) {
            warnKrita << "WARNING: loading layer style for" << layer->name() << "failed! It requests inexistent style:" << uuid;
        }
    }

    KisNodeSP child = root->firstChild();
    while (child) {
        assignAllLayerStyles(child);
        child = child->nextSibling();
    }
}
