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
#include <QUrl>
#include <QScopedPointer>

#include <KoColorSpace.h>

#include <KisDocument.h>
#include <KisMimeDatabase.h>
#include <KisPart.h>
#include <kis_image.h>
#include <kis_types.h>
#include <kis_node.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_layer.h>

#include "Krita.h"
#include "Node.h"
#include "Channel.h"
#include "ColorDepth.h"
#include "ColorModel.h"
#include "ColorProfile.h"
#include "Generator.h"
#include "Filter.h"
#include "Transformation.h"
#include "Selection.h"


struct Node::Private {
    Private() {}
    KisNodeSP node;
};

Node::Node(KisNodeSP node, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->node = node;
}

Node::~Node()
{
    delete d;
}

bool Node::alphaLocked() const
{
    if (!d->node) return false;
    KisPaintLayerSP paintLayer = qobject_cast<KisPaintLayer*>(d->node.data());
    if (paintLayer) {
        return paintLayer->alphaLocked();
    }
    return false;
}

void Node::setAlphaLocked(bool value)
{
    if (!d->node) return;
    KisPaintLayerSP paintLayer = qobject_cast<KisPaintLayer*>(d->node.data());
    if (paintLayer) {
        paintLayer->setAlphaLocked(value);
    }
}


QString Node::blendingMode() const
{
    if (!d->node) return QString();

    return d->node->compositeOpId();
}

void Node::setBlendingMode(QString value)
{
    if (!d->node) return;
    d->node->setCompositeOpId(value);
}


QList<Channel*> Node::channels() const
{
    if (!d->node) return QList<Channel*>();
    return QList<Channel*>();
}

void Node::setChannels(QList<Channel*> value)
{
    if (!d->node) return;
}


QList<Node*> Node::childNodes() const
{
    QList<Node*> nodes;
    if (d->node) {
        int childCount = d->node->childCount();
        for (int i = 0; i < childCount; ++i) {
            nodes << new Node(d->node->at(i));
        }
    }
    return nodes;
}

void Node::setChildNodes(QList<Node*> value)
{
    if (!d->node) return;
}


ColorDepth* Node::colorDepth() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setColorDepth(ColorDepth* value)
{
    if (!d->node) return;
}


QString Node::colorLabel() const
{
    if (!d->node) return QString();
    return QString();
}

void Node::setColorLabel(QString value)
{
    if (!d->node) return;
}


ColorModel* Node::colorModel() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setColorModel(ColorModel* value)
{
    if (!d->node) return;
}


ColorProfile* Node::colorProfile() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setColorProfile(ColorProfile* value)
{
    if (!d->node) return;
}


bool Node::inheritAlpha() const
{
    if (!d->node) return false;
    return false;
}

void Node::setInheritAlpha(bool value)
{
    if (!d->node) return;
}


bool Node::locked() const
{
    if (!d->node) return false;
    return false;
}

void Node::setLocked(bool value)
{
    if (!d->node) return;
}


QString Node::name() const
{
    if (!d->node) return QString();
    return d->node->name();
}

void Node::setName(QString value)
{
    if (!d->node) return;
}


int Node::opacity() const
{
    if (!d->node) return 0;
    return d->node->opacity();
}

void Node::setOpacity(int value)
{
    if (!d->node) return;
    d->node->setOpacity(value);
}


Node* Node::parentNode() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setParentNode(Node* value)
{
    if (!d->node) return;
}


QString Node::type() const
{
    if (!d->node) return QString();
    return QString();
}

void Node::setType(QString value)
{
    if (!d->node) return;
}


bool Node::visible() const
{
    if (!d->node) return false;
    return false;
}

void Node::setVisible(bool value)
{
    if (!d->node) return;
}


InfoObject* Node::metaDataInfo() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setMetaDataInfo(InfoObject* value)
{
    if (!d->node) return;
}


Generator* Node::generator() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setGenerator(Generator* value)
{
    if (!d->node) return;
}


Filter* Node::filter() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setFilter(Filter* value)
{
    if (!d->node) return;
}


Transformation* Node::transformation() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setTransformation(Transformation* value)
{
    if (!d->node) return;
}


Selection* Node::selection() const
{
    if (!d->node) return 0;
    return 0;
}

void Node::setSelection(Selection* value)
{
    if (!d->node) return;
}


QString Node::fileName() const
{
    if (!d->node) return QString();
    return QString();
}

void Node::setFileName(QString value)
{
    if (!d->node) return;
}


QByteArray Node::pixelData() const
{
    if (!d->node) return QByteArray();
    return QByteArray();
}

void Node::setPixelData(QByteArray value)
{
    if (!d->node) return;
}

QRect Node::bounds() const
{
    if (!d->node) return QRect();
    return d->node->exactBounds();
}

void Node::move(int x, int y)
{
    if (!d->node) return;
}

void Node::moveToParent(Node *parent)
{
    if (!d->node) return;
}

void Node::remove()
{
    if (!d->node) return;
}

Node* Node::duplicate()
{
    if (!d->node) return 0;
    return 0;
}

bool Node::save(const QString &filename, double xRes, double yRes)
{
    if (!d->node) return false;
    if (filename.isEmpty()) return false;

    KisPaintDeviceSP projection = d->node->projection();
    QRect bounds = d->node->exactBounds();

    QString mimefilter = KisMimeDatabase::mimeTypeForFile(filename);;
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    KisImageSP dst = new KisImage(doc->createUndoStore(),
                                  bounds.width(),
                                  bounds.height(),
                                  projection->compositionSourceColorSpace(),
                                  d->node->name());
    dst->setResolution(xRes, yRes);
    doc->setFileBatchMode(Krita::instance()->batchmode());
    doc->setCurrentImage(dst);
    KisPaintLayer* paintLayer = new KisPaintLayer(dst, "paint device", d->node->opacity());
    paintLayer->paintDevice()->makeCloneFrom(projection, bounds);
    dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));
    dst->initialRefreshGraph();
    doc->setOutputMimeType(mimefilter.toLatin1());

    return doc->exportDocument(QUrl::fromLocalFile(filename));
}
