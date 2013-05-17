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
#include "kis_layer.h"
#include "kis_shape_layer.h"
#include "kis_paint_layer.h"
#include "kis_doc2.h"
#include "kis_shape_controller.h"


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
        return m_node->projection()->convertToQImage(cfg.displayProfile(),
                                                     KoColorConversionTransformation::InternalRenderingIntent,
                                                     KoColorConversionTransformation::InternalConversionFlags);
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

void KisMimeData::initializeExternalNode(KisNodeSP &node,
                                         KisImageWSP image,
                                         KisShapeController *shapeController)
{
    // layers store a link to the image, so update it
    KisLayer *layer = dynamic_cast<KisLayer*>(node.data());
    if (layer) {
        layer->setImage(image);
    }
    KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(node.data());
    if (shapeLayer) {
        KoShapeContainer * parentContainer =
            dynamic_cast<KoShapeContainer*>(shapeController->shapeForNode(image->root()));

        KisShapeLayer *shapeLayer2 = new KisShapeLayer(parentContainer, shapeController, image, node->name(), node->opacity());
        QList<KoShape *> shapes = shapeLayer->shapes();
        shapeLayer->removeAllShapes();
        foreach(KoShape *shape, shapes) {
            shapeLayer2->addShape(shape);
        }
        node = shapeLayer2;
    }
}

KisNodeSP KisMimeData::tryLoadInternalNode(const QMimeData *data,
                                           KisImageWSP image,
                                           KisShapeController *shapeController,
                                           bool /* IN-OUT */ &copyNode)
{
    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data);
    KisNodeSP node = mimedata ? mimedata->node() : 0;

    if (node && (copyNode || node->graphListener() != image.data())) {
        node = node->clone();
        initializeExternalNode(node, image, shapeController);
        copyNode = true;
    }

    return node;
}

KisNodeSP KisMimeData::loadNode(const QMimeData *data,
                                const QRect &imageBounds,
                                const QPoint &preferredCenter,
                                bool forceRecenter, KisImageWSP image,
                                KisShapeController *shapeController)
{
    bool alwaysRecenter = false;
    KisNodeSP node;

    if (data->hasFormat("application/x-krita-node")) {
        QByteArray ba = data->data("application/x-krita-node");

        KisDoc2 tempDoc;
        tempDoc.loadNativeFormatFromStore(ba);

        KisImageWSP tempImage = tempDoc.image();
        node = tempImage->root()->firstChild();
        tempImage->removeNode(node);

        initializeExternalNode(node, image, shapeController);
    }
    else if (data->hasImage()) {
        QImage qimage = qvariant_cast<QImage>(data->imageData());

        KisPaintDeviceSP device = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
        device->convertFromQImage(qimage, 0);
        node = new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, device);

        alwaysRecenter = true;
    }

    if (node) {
        QRect bounds = node->projection()->exactBounds();
        if (alwaysRecenter || forceRecenter ||
            (!imageBounds.contains(bounds) &&
             !imageBounds.intersects(bounds))) {

            QPoint pt = preferredCenter - bounds.center();
            node->setX(pt.x());
            node->setY(pt.y());
        }
    }

    return node;
}
