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
#include "Node.h"
#include "Channel.h"
#include "ColorDepth.h"
#include "ColorModel.h"
#include "ColorProfile.h"
#include "Generator.h"
#include "Filter.h"
#include "Transformation.h"
#include "Selection.h"

#include <kis_types.h>
#include <kis_node.h>

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
    return QList<Node*>();
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
    return QString();
}

void Node::setName(QString value)
{
}


int Node::opacity() const
{
    return 0;
}

void Node::setOpacity(int value)
{
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

bool Node::save(const QString &filename) 
{
    return false;
}
