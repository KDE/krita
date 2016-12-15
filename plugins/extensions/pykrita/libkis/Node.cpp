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
    return false;
}

void Node::setAlphaLocked(bool value)
{
}


QString Node::blendingMode() const
{
    return QString();
}

void Node::setBlendingMode(QString value)
{
}


QList<Channel*> Node::channels() const
{
    return QList<Channel*>();
}

void Node::setChannels(QList<Channel*> value)
{
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
}


ColorDepth* Node::colorDepth() const
{
    return 0;
}

void Node::setColorDepth(ColorDepth* value)
{
}


QString Node::colorLabel() const
{
    return QString();
}

void Node::setColorLabel(QString value)
{
}


ColorModel* Node::colorModel() const
{
    return 0;
}

void Node::setColorModel(ColorModel* value)
{
}


ColorProfile* Node::colorProfile() const
{
    return 0;
}

void Node::setColorProfile(ColorProfile* value)
{
}


bool Node::inheritAlpha() const
{
    return false;
}

void Node::setInheritAlpha(bool value)
{
}


bool Node::locked() const
{
    return false;
}

void Node::setLocked(bool value)
{
}


QString Node::name() const
{
    return d->node->name();
}

void Node::setName(QString value)
{
}


int Node::opacity() const
{
    return d->node->opacity();
}

void Node::setOpacity(int value)
{
    d->node->setOpacity(value);
}


Node* Node::parentNode() const
{
    return 0;
}

void Node::setParentNode(Node* value)
{
}


QString Node::type() const
{
    return QString();
}

void Node::setType(QString value)
{
}


bool Node::visible() const
{
    return false;
}

void Node::setVisible(bool value)
{
}


InfoObject* Node::metaDataInfo() const
{
    return 0;
}

void Node::setMetaDataInfo(InfoObject* value)
{
}


Generator* Node::generator() const
{
    return 0;
}

void Node::setGenerator(Generator* value)
{
}


Filter* Node::filter() const
{
    return 0;
}

void Node::setFilter(Filter* value)
{
}


Transformation* Node::transformation() const
{
    return 0;
}

void Node::setTransformation(Transformation* value)
{
}


Selection* Node::selection() const
{
    return 0;
}

void Node::setSelection(Selection* value)
{
}


QString Node::fileName() const
{
    return QString();
}

void Node::setFileName(QString value)
{
}


QByteArray Node::pixelData() const
{
    return QByteArray();
}

void Node::setPixelData(QByteArray value)
{
}

void Node::move(int x, int y)
{
}

void Node::moveToParent(Node *parent)
{
}

void Node::remove()
{
}

Node* Node::duplicate()
{
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
