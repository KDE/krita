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
#include "kis_image.h"
#include "kis_doc2.h"

#include <KoStore.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QDomDocument>
#include <QDomElement>
#include <QTemporaryFile>

KisMimeData::KisMimeData(KisNodeSP node)
    : QMimeData()
    , m_node(node)
{
    Q_ASSERT(m_node);
}


KisNodeSP KisMimeData::node() const
{
    return m_node;
}

QStringList KisMimeData::formats () const
{
    QStringList f = QMimeData::formats();
    if (m_node) {
#if QT_VERSION  < 0x040800
        f << "application/x-krita-node"
          << "application/x-qt-image";
#else
        f << "application/x-krita-node";
#endif
    }
    return f;
}

QVariant KisMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    Q_ASSERT(m_node);
    if (mimetype == "application/x-qt-image") {
        KisConfig cfg;
        return m_node->projection()->convertToQImage(cfg.displayProfile(), KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
    }
    else if (mimetype == "application/x-krita-node"
             || mimetype == "application/zip") {

        KisNode *node = const_cast<KisNode*>(m_node.constData());

        QByteArray ba;
        QBuffer buf(&ba);
        KoStore *store = KoStore::createStore(&buf, KoStore::Write);
        Q_ASSERT(!store->bad());
        store->disallowNameExpansion();

        KisDoc2 doc;

        QRect rc = node->exactBounds();

        KisImageSP image = new KisImage(0, rc.width(), rc.height(), node->colorSpace(), node->name(), false);
        image->addNode(node->clone());
        doc.setCurrentImage(image);

        doc.saveNativeFormatCalligra(store);

#if 0
        QFile f("./KRITA_DROP_FILE.kra");
        f.open(QFile::WriteOnly);
        f.write(ba);
        f.flush();
        f.close();
#endif

        return ba;

    }
    else {
        return QMimeData::retrieveData(mimetype, preferredType);
    }
}
