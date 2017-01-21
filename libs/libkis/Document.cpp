/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Document.h"
#include <QPointer>
#include <QUrl>
#include <QDomDocument>

#include <KoColorSpaceConstants.h>
#include <KoXmlReader.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <KisPart.h>
#include <kis_paint_device.h>
#include <KisMainWindow.h>
#include <kis_node_manager.h>
#include <kis_node_selection_adapter.h>
#include <KisViewManager.h>
#include <kis_file_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_mask.h>
#include <kis_clone_layer.h>
#include <kis_group_layer.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_transparency_mask.h>
#include <kis_selection_mask.h>
#include <kis_effect_mask.h>
#include <kis_paint_layer.h>
#include <kis_generator_layer.h>
#include <kis_shape_layer.h>
#include <kis_filter_configuration.h>
#include <kis_selection.h>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversionTransformation.h>
#include <KoDocumentInfo.h>

#include <InfoObject.h>
#include <Node.h>
#include <Selection.h>

struct Document::Private {
    Private() {}
    QPointer<KisDocument> document;
    bool ownsDocument {false};
};

Document::Document(KisDocument *document, bool ownsDocument, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->document = document;
    d->ownsDocument = ownsDocument;
}

Document::~Document()
{
    qDebug() << "Document" << this << "deleted";
    if (d->ownsDocument) {
        KisPart::instance()->removeDocument(d->document);
        delete d->document;
    }

    delete d;
}

bool Document::batchmode() const
{
    if (!d->document) return false;
    return d->document->fileBatchMode();
}

void Document::setBatchmode(bool value)
{
    if (!d->document) return;
    d->document->setFileBatchMode(value);
}

Node *Document::activeNode() const
{
    QList<KisNodeSP> activeNodes;
    Q_FOREACH(QPointer<KisView> view, KisPart::instance()->views()) {
        if (view && view->document() == d->document) {
            activeNodes << view->currentNode();
        }
    }
    if (activeNodes.size() > 0) {
        return new Node(d->document->image(), activeNodes.first());
    }
    return new Node(d->document->image(), d->document->image()->root()->firstChild());
}

void Document::setActiveNode(Node* value)
{
    if (!value->node()) return;
    KisMainWindow *mainWin = KisPart::instance()->currentMainwindow();
    if (!mainWin) return;
    KisViewManager *viewManager = mainWin->viewManager();
    if (!viewManager) return;
    KisNodeManager *nodeManager = viewManager->nodeManager();
    if (!nodeManager) return;
    KisNodeSelectionAdapter *selectionAdapter = nodeManager->nodeSelectionAdapter();
    if (!selectionAdapter) return;
    selectionAdapter->setActiveNode(value->node());

}


QString Document::colorDepth() const
{
    if (!d->document) return "";
    return d->document->image()->colorSpace()->colorDepthId().id();
}

QString Document::colorModel() const
{
    if (!d->document) return "";
    return d->document->image()->colorSpace()->colorModelId().id();
}

QString Document::colorProfile() const
{
    if (!d->document) return "";
    return d->document->image()->colorSpace()->profile()->name();
}

bool Document::setColorProfile(const QString &value)
{
    if (!d->document) return false;
    if (!d->document->image()) return false;
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(value);
    if (!profile) return false;
    d->document->image()->lock();
    bool retval = d->document->image()->assignImageProfile(profile);
    d->document->image()->unlock();
    d->document->image()->setModified();
    d->document->image()->initialRefreshGraph();
    return retval;
}

bool Document::setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile)
{
    if (!d->document) return false;
    if (!d->document->image()) return false;
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, colorProfile);
    if (!colorSpace) return false;
    d->document->image()->lock();
    d->document->image()->convertImageColorSpace(colorSpace,
                                                 KoColorConversionTransformation::IntentPerceptual,
                                                 KoColorConversionTransformation::HighQuality | KoColorConversionTransformation::NoOptimization);
    d->document->image()->unlock();
    d->document->image()->setModified();
    d->document->image()->initialRefreshGraph();
    return true;
}


QString Document::documentInfo() const
{
    QDomDocument doc = KisDocument::createDomDocument("document-info"
                                                      /*DTD name*/, "document-info" /*tag name*/, "1.1");
    doc = d->document->documentInfo()->save(doc);
    return doc.toString();
}

void Document::setDocumentInfo(const QString &document)
{
    KoXmlDocument doc = KoXmlDocument(true);
    QString errorMsg;
    int errorLine, errorColumn;
    doc.setContent(document, &errorMsg, &errorLine, &errorColumn);
    d->document->documentInfo()->load(doc);
}

QString Document::fileName() const
{
    if (!d->document) return QString::null;
    return d->document->url().toLocalFile();
}

void Document::setFileName(QString value)
{
    if (!d->document) return;
    d->document->setUrl(QUrl::fromLocalFile(value));
}


int Document::height() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;
    return image->height();
}

void Document::setHeight(int value)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    QRect rc = image->bounds();
    rc.setHeight(value);
    image->resizeImage(rc);
}


QString Document::name() const
{
    if (!d->document) return "";
    return d->document->documentInfo()->aboutInfo("title");
}

void Document::setName(QString value)
{
    if (!d->document) return;
    d->document->documentInfo()->setAboutInfo("title", value);
}


int Document::resolution() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;

    return qRound(d->document->image()->xRes() * 72);
}

void Document::setResolution(int value)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;

    d->document->image()->setResolution(value / 72.0, value / 72.0);
}


Node *Document::rootNode() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;

    return new Node(image, image->root());
}

Selection *Document::selection() const
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    if (!d->document->image()->globalSelection()) return 0;
    return new Selection(d->document->image()->globalSelection());
}

void Document::setSelection(Selection* value)
{
    if (!d->document) return;
    if (!d->document->image()) return;
    d->document->image()->setGlobalSelection(value->selection());
}


int Document::width() const
{
    if (!d->document) return 0;
    KisImageSP image = d->document->image();
    if (!image) return 0;
    return image->width();
}

void Document::setWidth(int value)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    QRect rc = image->bounds();
    rc.setWidth(value);
    image->resizeImage(rc);
}

double Document::xRes() const
{
    if (!d->document) return 0.0;
    if (!d->document->image()) return 0.0;
    return d->document->image()->xRes();
}

void Document::setXRes(double xRes) const
{
    if (!d->document) return;
    if (!d->document->image()) return;
    d->document->image()->setResolution(xRes, d->document->image()->yRes());
}

double Document::yRes() const
{
    if (!d->document) return 0.0;
    if (!d->document->image()) return 0.0;
    return d->document->image()->yRes();
}

void Document::setyRes(double yRes) const
{
    if (!d->document) return;
    if (!d->document->image()) return;
    d->document->image()->setResolution(d->document->image()->xRes(), yRes);
}


QByteArray Document::pixelData(int x, int y, int w, int h) const
{
    QByteArray ba;

    if (!d->document) return ba;
    KisImageSP image = d->document->image();
    if (!image) return ba;

    KisPaintDeviceSP dev = image->projection();
    quint8 *data = new quint8[w * h * dev->pixelSize()];
    dev->readBytes(data, x, y, w, h);
    ba = QByteArray((const char*)data, (int)(w * h * dev->pixelSize()));
    delete[] data;
    return ba;
}

bool Document::close()
{
    if (d->ownsDocument) {
        KisPart::instance()->removeDocument(d->document);

    }
    bool retval = d->document->closeUrl(false);
    if (d->ownsDocument) {
        delete d->document;
    }
    return retval;
}

void Document::crop(int x, int y, int w, int h)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    QRect rc(x, y, w, h);
    image->cropImage(rc);
}

bool Document::exportImage(const QString &filename, const InfoObject &exportConfiguration)
{
    if (!d->document) return false;
    return d->document->exportDocument(QUrl::fromLocalFile(filename));
}

void Document::flatten()
{
    if (!d->document) return;
    if (!d->document->image()) return;
    d->document->image()->lock();
    d->document->image()->flatten();
    d->document->image()->unlock();
    d->document->image()->setModified();
    d->document->image()->initialRefreshGraph();
}

void Document::resizeImage(int w, int h)
{
    if (!d->document) return;
    KisImageSP image = d->document->image();
    if (!image) return;
    QRect rc = image->bounds();
    rc.setWidth(w);
    rc.setHeight(h);
    image->resizeImage(rc);
}

bool Document::save()
{
    if (!d->document) return false;
    return d->document->save();
}

bool Document::saveAs(const QString &filename)
{
    if (!d->document) return false;
    return d->document->saveAs(QUrl::fromLocalFile(filename));
}

void Document::openView()
{
    // UNIMPLEMENTED
}

Node* Document::createNode(const QString &name, const QString &nodeType)
{
    if (!d->document) return 0;
    if (!d->document->image()) return 0;
    KisImageSP image = d->document->image();

    Node *node = 0;

    if (nodeType == "paintlayer") {
        node = new Node(image, new KisPaintLayer(image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType == "grouplayer") {
        node = new Node(image, new KisGroupLayer(image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType == "filelayer") {
        node = new Node(image, new KisFileLayer(image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType == "filterlayer") {
        node = new Node(image, new KisAdjustmentLayer(image, name, 0, 0));
    }
    else if (nodeType == "filllayer") {
        node = new Node(image, new KisGeneratorLayer(image, name, 0, 0));
    }
    else if (nodeType == "clonelayer") {
        node = new Node(image, new KisCloneLayer(0, image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType == "vectorlayer") {
        node = new Node(image, new KisShapeLayer(d->document->shapeController(), image, name, OPACITY_OPAQUE_U8));
    }
    else if (nodeType == "transparencymask") {
        node = new Node(image, new KisTransparencyMask());
    }
    else if (nodeType == "filtermask") {
        node = new Node(image, new KisFilterMask());
    }
    else if (nodeType == "transformmask") {
        node = new Node(image, new KisTransformMask());
    }
    else if (nodeType == "localselectionmask") {
        node = new Node(image, new KisSelectionMask(image));
    }

    return node;
}

Node *Document::mergeDown(Node *node)
{
    if (!d->document) return 0;
    // UNINMPLEMENTED
    return 0;
}

QPointer<KisDocument> Document::document() const
{
    return d->document;
}



