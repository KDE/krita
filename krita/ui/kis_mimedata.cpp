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

#include <QApplication>
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
        f << "application/x-krita-node"
          << "application/x-krita-node-url"
          << "application/x-qt-image"
          << "application/zip"
          << "application/x-krita-node-internal-pointer";
    }
    return f;
}

QByteArray serializeToByteArray(KisNodeSP node)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    KoStore *store = KoStore::createStore(&buffer, KoStore::Write);
    Q_ASSERT(!store->bad());
    store->disallowNameExpansion();

    KisDoc2 doc;

    QRect rc = node->exactBounds();

    KisImageSP image = new KisImage(0, rc.width(), rc.height(), node->colorSpace(), node->name(), false);
    image->addNode(node->clone());
    doc.setCurrentImage(image);

    doc.saveNativeFormatCalligra(store);

    return byteArray;
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
    else if (mimetype == "application/x-krita-node" ||
             mimetype == "application/zip") {

        QByteArray ba = serializeToByteArray(m_node);
        return ba;

    } else if (mimetype == "application/x-krita-node-url") {

        QByteArray ba = serializeToByteArray(m_node);

        QString temporaryPath =
            QDir::tempPath() + QDir::separator() +
            QString("krita_tmp_dnd_layer_%1_%2.kra")
            .arg(QApplication::applicationPid())
            .arg(qrand());


        QFile file(temporaryPath);
        file.open(QFile::WriteOnly);
        file.write(ba);
        file.flush();
        file.close();

        return QUrl(temporaryPath).toEncoded();
    } else if (mimetype == "application/x-krita-node-internal-pointer") {

        QDomDocument doc("krita_internal_node_pointer");
        QDomElement element = doc.createElement("pointer");
        element.setAttribute("application_pid", (qint64)QApplication::applicationPid());
        element.setAttribute("pointer_value", (qint64)m_node.data());
        doc.appendChild(element);

        return doc.toByteArray();

    } else {
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
    // Qt 4.7 way
    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data);
    KisNodeSP node = mimedata ? mimedata->node() : 0;

    // Qt 4.8 way
    if (!node && data->hasFormat("application/x-krita-node-internal-pointer")) {
        QByteArray nodeXml = data->data("application/x-krita-node-internal-pointer");

        QDomDocument doc;
        doc.setContent(nodeXml);

        QDomElement element = doc.documentElement();
        qint64 pid = element.attribute("application_pid").toLongLong();
        qint64 pointerValue = element.attribute("pointer_value").toLongLong();

        if (pid == QApplication::applicationPid() &&
            pointerValue) {

            node = reinterpret_cast<KisNode*>(pointerValue);
        }
    }

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
        bool result = tempDoc.loadNativeFormatFromStore(ba);

        if (result) {
            KisImageWSP tempImage = tempDoc.image();
            node = tempImage->root()->firstChild();
            tempImage->removeNode(node);

            initializeExternalNode(node, image, shapeController);
        }
    }

    if (!node && data->hasFormat("application/x-krita-node-url")) {
        QByteArray ba = data->data("application/x-krita-node-url");
        QString localFile = QUrl::fromEncoded(ba).toLocalFile();

        KisDoc2 tempDoc;
        bool result = tempDoc.loadNativeFormat(localFile);

        if (result) {
            KisImageWSP tempImage = tempDoc.image();
            node = tempImage->root()->firstChild();
            tempImage->removeNode(node);

            initializeExternalNode(node, image, shapeController);
        }

        QFile::remove(localFile);
    }

    if (!node && data->hasImage()) {
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
