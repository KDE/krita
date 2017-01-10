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
#include <KisDocument.h>
#include <kis_image.h>
#include <KisPart.h>
#include <kis_paint_device.h>

#include <InfoObject.h>
#include <Node.h>

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
}


ColorDepth* Document::colorDepth() const
{
    return 0;
}

void Document::setColorDepth(ColorDepth* value)
{
}


ColorModel* Document::colorModel() const
{
    return 0;
}

void Document::setColorModel(ColorModel* value)
{
}


ColorProfile* Document::colorProfile() const
{
    return 0;
}

void Document::setColorProfile(ColorProfile* value)
{
}


InfoObject* Document::documentInfo() const
{
    return 0;
}

void Document::setDocumentInfo(InfoObject* value)
{
}


QString Document::fileName() const
{
    if (!d->document) return QString::null;
    return d->document->url().toLocalFile();
}

void Document::setFileName(QString value)
{
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


InfoObject* Document::metaData() const
{
    return 0;
}

void Document::setMetaData(InfoObject* value)
{
}


QString Document::name() const
{
    return QString();
}

void Document::setName(QString value)
{
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
    return 0;
}

void Document::setSelection(Selection* value)
{
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


QByteArray Document::pixelData() const
{
    QByteArray ba;

    if (!d->document) return ba;
    KisImageSP image = d->document->image();
    if (!image) return ba;

    KisPaintDeviceSP dev = image->projection();
    quint8 *data = new quint8[image->width() * image->height() * dev->pixelSize()];
    dev->readBytes(data, 0, 0, image->width(), image->height());
    ba = QByteArray((const char*)data, (int)(image->width() * image->height() * dev->pixelSize()));
    delete[] data;
    return ba;
}

bool Document::close()
{
    if (d->ownsDocument) {
        KisPart::instance()->removeDocument(d->document);
    }
    return d->document->closeUrl(false);
}

bool Document::convert(const QString &colorModel, const ColorProfile *profile)
{
    return false;
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
}

Node* Document::createNode(const QString &name, const QString &nodeType)
{
    return 0;
}

QPointer<KisDocument> Document::document() const
{
    return d->document;
}



