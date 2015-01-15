/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include "KisDocument.h"
#include "kis_shape_controller.h"
#include "KisPart.h"

#include <KoProperties.h>
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
#include <QDesktopWidget>

KisMimeData::KisMimeData(QList<KisNodeSP> nodes)
    : QMimeData()
    , m_nodes(nodes)
{
    Q_ASSERT(m_nodes.size() > 0);
}


QList<KisNodeSP> KisMimeData::nodes() const
{
    return m_nodes;
}

QStringList KisMimeData::formats () const
{
    QStringList f = QMimeData::formats();
    if (m_nodes.size() > 0) {
        f << "application/x-krita-node"
          << "application/x-krita-node-url"
          << "application/x-qt-image"
          << "application/zip"
          << "application/x-krita-node-internal-pointer";
    }
    return f;
}

KisDocument *createDocument(QList<KisNodeSP> nodes)
{
    KisDocument *doc = KisPart::instance()->createDocument();
    QRect rc;
    foreach(KisNodeSP node, nodes) {
        rc |= node->exactBounds();
    }

    KisImageSP image = new KisImage(0, rc.width(), rc.height(), nodes.first()->colorSpace(), nodes.first()->name(), false);

    foreach(KisNodeSP node, nodes) {
        image->addNode(node->clone());
    }

    doc->setCurrentImage(image);

    return doc;
}

QByteArray serializeToByteArray(QList<KisNodeSP> nodes)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    KoStore *store = KoStore::createStore(&buffer, KoStore::Write);
    Q_ASSERT(!store->bad());
    
    KisDocument *doc = createDocument(nodes);
    doc->saveNativeFormatCalligra(store);
    delete doc;

    return byteArray;
}

QVariant KisMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    Q_ASSERT(m_nodes.size() > 0);

    if (mimetype == "application/x-qt-image") {
        KisConfig cfg;

        KisDocument *doc = createDocument(m_nodes);
        doc->image()->refreshGraph();
        doc->image()->waitForDone();

        return doc->image()->projection()->convertToQImage(cfg.displayProfile(QApplication::desktop()->screenNumber(qApp->activeWindow())),
                                                           KoColorConversionTransformation::InternalRenderingIntent,
                                                           KoColorConversionTransformation::InternalConversionFlags);
    }
    else if (mimetype == "application/x-krita-node" ||
             mimetype == "application/zip") {

        QByteArray ba = serializeToByteArray(m_nodes);
        return ba;

    }
    else if (mimetype == "application/x-krita-node-url") {

        QByteArray ba = serializeToByteArray(m_nodes);

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
    }
    else if (mimetype == "application/x-krita-node-internal-pointer") {

        QDomDocument doc("krita_internal_node_pointer");
        QDomElement root = doc.createElement("pointer");
        root.setAttribute("application_pid", (qint64)QApplication::applicationPid());
        doc.appendChild(root);

        foreach(KisNodeSP node, m_nodes) {
            QDomElement element = doc.createElement("node");
            element.setAttribute("pointer_value", (qint64)node.data());
            root.appendChild(element);
        }

        return doc.toByteArray();

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
        KisShapeLayer *shapeLayer2 = new KisShapeLayer(shapeController, image, node->name(), node->opacity());
        QList<KoShape *> shapes = shapeLayer->shapes();
        shapeLayer->removeAllShapes();
        foreach(KoShape *shape, shapes) {
            shapeLayer2->addShape(shape);
        }
        node = shapeLayer2;
    }
}

QList<KisNodeSP> KisMimeData::tryLoadInternalNodes(const QMimeData *data,
                                                   KisImageWSP image,
                                                   KisShapeController *shapeController,
                                                   bool /* IN-OUT */ &copyNode)
{
    QList<KisNodeSP> nodes;
    // Qt 4.7 way
    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data);
    if (mimedata) {
        nodes = mimedata->nodes();
    }

    // Qt 4.8 way
    if (nodes.isEmpty() && data->hasFormat("application/x-krita-node-internal-pointer")) {
        QByteArray nodeXml = data->data("application/x-krita-node-internal-pointer");

        QDomDocument doc;
        doc.setContent(nodeXml);

        QDomElement element = doc.documentElement();
        qint64 pid = element.attribute("application_pid").toLongLong();

        if (pid == QApplication::applicationPid()) {

            QDomNode n = element.firstChild();
            while (!n.isNull()) {
                QDomElement e = n.toElement();
                if (!e.isNull()) {
                    qint64 pointerValue = e.attribute("pointer_value").toLongLong();
                    if (pointerValue) {
                        nodes << reinterpret_cast<KisNode*>(pointerValue);
                    }
                }
                n = n.nextSibling();
            }
        }
    }

    if (!nodes.isEmpty() && (copyNode || nodes.first()->graphListener() != image.data())) {
        QList<KisNodeSP> clones;
        copyNode = true;
        foreach(KisNodeSP node, nodes) {
            node = node->clone();
            initializeExternalNode(node, image, shapeController);
            clones << node;
        }
        nodes = clones;
    }
    return nodes;
}

QList<KisNodeSP> KisMimeData::loadNodes(const QMimeData *data,
                                        const QRect &imageBounds,
                                        const QPoint &preferredCenter,
                                        bool forceRecenter, KisImageWSP image,
                                        KisShapeController *shapeController)
{
    bool alwaysRecenter = false;
    QList<KisNodeSP> nodes;

    if (data->hasFormat("application/x-krita-node")) {
        QByteArray ba = data->data("application/x-krita-node");

        KisDocument *tempDoc = KisPart::instance()->createDocument();
        bool result = tempDoc->loadNativeFormatFromByteArray(ba);

        if (result) {
            KisImageWSP tempImage = tempDoc->image();
            foreach(KisNodeSP node, tempImage->root()->childNodes(QStringList(), KoProperties())) {
                nodes << node;
                tempImage->removeNode(node);
                initializeExternalNode(node, image, shapeController);
            }
        }
        delete tempDoc;
    }

    if (nodes.isEmpty() && data->hasFormat("application/x-krita-node-url")) {
        QByteArray ba = data->data("application/x-krita-node-url");
        QString localFile = QUrl::fromEncoded(ba).toLocalFile();

        KisDocument *tempDoc = KisPart::instance()->createDocument();
        bool result = tempDoc->loadNativeFormat(localFile);

        if (result) {
            KisImageWSP tempImage = tempDoc->image();
            foreach(KisNodeSP node, tempImage->root()->childNodes(QStringList(), KoProperties())) {
                nodes << node;
                tempImage->removeNode(node);
                initializeExternalNode(node, image, shapeController);
            }
        }
        delete tempDoc;

        QFile::remove(localFile);
    }

    if (nodes.isEmpty() && data->hasImage()) {
        QImage qimage = qvariant_cast<QImage>(data->imageData());

        KisPaintDeviceSP device = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
        device->convertFromQImage(qimage, 0);
        nodes << new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, device);

        alwaysRecenter = true;
    }

    if (!nodes.isEmpty()) {
        foreach(KisNodeSP node, nodes) {
            QRect bounds = node->projection()->exactBounds();
            if (alwaysRecenter || forceRecenter ||
                    (!imageBounds.contains(bounds) &&
                     !imageBounds.intersects(bounds))) {

                QPoint pt = preferredCenter - bounds.center();
                node->setX(pt.x());
                node->setY(pt.y());
            }
        }
    }

    return nodes;
}
