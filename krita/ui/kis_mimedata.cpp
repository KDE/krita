/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_mimedata.h"
#include "kis_config.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "kis_shared_ptr.h"
#include "kra/kis_kra_savexml_visitor.h"
#include "kra/kis_kra_save_visitor.h"

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QDomDocument>
#include <QDomElement>

KisMimeData::KisMimeData() :
    QMimeData()
{
}

void KisMimeData::setNode(KisNodeSP node)
{
    m_node = node;
}

KisNodeSP KisMimeData::node() const
{
    return m_node;
}

QStringList KisMimeData::formats () const
{
    QStringList f = QMimeData::formats();
    if (m_node) {
        f << "application/x-krita-node-pointer"
          << "application/x-krita-node"
          << "application/x-qt-image";
    }
    return f;
}

QVariant KisMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    if (mimetype == "application/x-qt-image") {
        KisConfig cfg;
        QString monitorProfileName = cfg.monitorProfile();
        const KoColorProfile *monitorProfile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
        return m_node->paintDevice()->convertToQImage(monitorProfile);
    }
    else if (mimetype == "application/x-krita-node") {

        KisNode *node = const_cast<KisNode*>(m_node.constData());

        QByteArray ba;
        QBuffer buf(&ba);
        KoStore *store = KoStore::createStore(&buf, KoStore::Write);

        store->open("layer.xml");
        KoStoreDevice dev(store);

        QDomDocument layerdoc;
        QDomElement nodeElement = layerdoc.createElement("NODE");
        quint32 count = 1;
        KisSaveXmlVisitor vxml(layerdoc, nodeElement, count, true);
        node->accept(vxml);

        QByteArray s = layerdoc.toByteArray();
        dev.write(s.constData(), s.size());
        store->close();

        KisKraSaveVisitor vbinary(store, count, node->name(), vxml.nodeFileNames());
        node->accept(vbinary);
        delete store;

        return ba;

    }
    else if (mimetype == "application/x-krita-node-pointer") {
        return qVariantFromValue(qulonglong(m_node.constData()));
    }
    else {
        return QMimeData::retrieveData(mimetype, preferredType);
    }
}
