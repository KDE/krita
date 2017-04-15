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
#include <kis_colorspace_convert_visitor.h>
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
};

Document::Document(KisDocument *document, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->document = document;
}

Document::~Document()
{
    delete d;
}

bool Document::operator==(const Document &other) const
{
    return (d->document == other.d->document);
}

bool Document::operator!=(const Document &other) const
{
    return !(operator==(other));
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
    if (viewManager->document() != d->document) return;
    KisNodeManager *nodeManager = viewManager->nodeManager();
    if (!nodeManager) return;
    KisNodeSelectionAdapter *selectionAdapter = nodeManager->nodeSelectionAdapter();
    if (!selectionAdapter) return;
    selectionAdapter->setActiveNode(value->node());

}

QList<Node *> Document::topLevelNodes() const
{
    if (!d->document) return QList<Node *>();
    Node n(d->document->image(), d->document->image()->rootLayer());
    return n.childNodes();
}


Node *Document::nodeByName(const QString &name) const
{
    if (!d->document) return 0;
    KisNodeSP node = d->document->image()->rootLayer()->findChildByName(name);
    return new Node(d->document->image(), node);
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
    bool retval = d->document->image()->assignImageProfile(profile);
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

    d->document->image()->convertImageColorSpace(colorSpace,
                                                 KoColorConversionTransformation::IntentPerceptual,
                                                 KoColorConversionTransformation::HighQuality | KoColorConversionTransformation::NoOptimization);

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
    if (!d->document->image()) return;
    resizeImage(d->document->image()->width(), value);
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
    if (value) {
        d->document->image()->setGlobalSelection(value->selection());
    }
    else {
        d->document->image()->setGlobalSelection(0);
    }
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
    if (!d->document->image()) return;
    resizeImage(value, d->document->image()->height());
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

void Document::setYRes(double yRes) const
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
    ba.resize(w * h * dev->pixelSize());
    dev->readBytes(reinterpret_cast<quint8*>(ba.data()), x, y, w, h);
    return ba;
}

bool Document::close()
{
    bool retval = d->document->closeUrl(false);
    Q_FOREACH(KisView *view, KisPart::instance()->views()) {
        if (view->document() == d->document) {
            view->close();
            view->deleteLater();
        }
    }

    d->document->deleteLater();
    d->document = 0;
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
    return d->document->exportDocument(QUrl::fromLocalFile(filename), exportConfiguration.configuration());
}

void Document::flatten()
{
    if (!d->document) return;
    if (!d->document->image()) return;
    d->document->image()->flatten();
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
    return d->document->save(true, 0);
}

bool Document::saveAs(const QString &filename)
{
    if (!d->document) return false;
    return d->document->saveAs(QUrl::fromLocalFile(filename), true);
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
    else if (nodeType == "selectionmask") {
        node = new Node(image, new KisSelectionMask(image));
    }
    return node;
}

QImage Document::projection(int x, int y, int w, int h) const
{
    if (!d->document || !d->document->image()) return QImage();
    return d->document->image()->convertToQImage(x, y, w, h, 0);
}

QImage Document::thumbnail(int w, int h) const
{
    if (!d->document || !d->document->image()) return QImage();
    return d->document->generatePreview(QSize(w, h)).toImage();
}


void Document::lock()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->barrierLock();
}

void Document::unlock()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->unlock();
}

void Document::waitForDone()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->waitForDone();
}

bool Document::tryBarrierLock()
{
    if (!d->document || !d->document->image()) return false;
    return d->document->image()->tryBarrierLock();
}

bool Document::isIdle()
{
    if (!d->document || !d->document->image()) return false;
    return d->document->image()->isIdle();
}

void Document::refreshProjection()
{
    if (!d->document || !d->document->image()) return;
    d->document->image()->refreshGraph();
}

QPointer<KisDocument> Document::document() const
{
    return d->document;
}
