/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "KoColorConversionSystem.h"

#include <QHash>
#include <QString>

#include <kdebug.h>

#include "KoColorConversionTransformationFactory.h"
#include "KoColorSpace.h"

struct KoColorConversionSystem::Node {
    Node() : isInitialized(false), colorSpaceFactory(0) {}
    void init( const KoColorSpaceFactory* _colorSpaceFactory)
    {
        Q_ASSERT(not isInitialized);
        isInitialized = true;
        isIcc = _colorSpaceFactory->isIcc();
        isHdr = _colorSpaceFactory->isHdr();
        colorSpaceFactory = _colorSpaceFactory;
    }
    QString modelId;
    QString depthId;
    bool isIcc;
    bool isHdr;
    bool isInitialized;
    QList<Vertex*> outputVertexes;
    const KoColorSpaceFactory* colorSpaceFactory;
};

struct KoColorConversionSystem::Vertex {
    Vertex(Node* _srcNode, Node* _dstNode) : srcNode(_srcNode), dstNode(_dstNode), factoryFromSrc(0), factoryFromDst(0) {}
    ~Vertex()
    {
        delete factoryFromSrc;
        delete factoryFromDst;
    }
    Node* srcNode;
    Node* dstNode;
    KoColorConversionTransformationFactory* factoryFromSrc; // Factory provided by the destination node
    KoColorConversionTransformationFactory* factoryFromDst; // Factory provided by the destination node
};

struct KoColorConversionSystem::NodeKey {
    NodeKey(QString _modelId, QString _depthId) : modelId(_modelId), depthId(_depthId)
    {}
    bool operator==(const KoColorConversionSystem::NodeKey& rhs) const
    {
        return modelId == rhs.modelId && depthId == rhs.depthId;
    }
    QString modelId;
    QString depthId;
};

uint qHash(const KoColorConversionSystem::NodeKey &key)
{
    return qHash(key.modelId) + qHash(key.depthId);
}

struct KoColorConversionSystem::Private {
    QHash<NodeKey, Node*> graph;
    QList<Vertex*> vertexes;
};


KoColorConversionSystem::KoColorConversionSystem() : d(new Private)
{
    
}

KoColorConversionSystem::~KoColorConversionSystem()
{
    QList<Node*> nodes = d->graph.values();
    foreach(Node* node, nodes)
    {
        delete node;
    }
    foreach(Vertex* vertex, d->vertexes)
    {
        delete vertex;
    }
    delete d;
}

void KoColorConversionSystem::insertColorSpace(const KoColorSpaceFactory* csf)
{
    kDebug() << "Inserting color space " << csf->name() << " (" << csf->id() << ") Model: " << csf->colorModelId() << " Depth: " << csf->colorDepthId() << " into the CCS";
    QString modelId = csf->colorModelId().id();
    QString depthId = csf->colorDepthId().id();
    NodeKey key(modelId, depthId);
    Node* csNode = nodeFor(key);
    Q_ASSERT(csNode);
    csNode->init(csf);
    if(csNode->isIcc)
    { // Construct a link between this color space and all other ICC color space
        QList<Node*> nodes = d->graph.values();
        foreach(Node* node, nodes)
        {
            if(node->isIcc and node->isInitialized and node != csNode)
            {
                // Create the vertex from 1 to 2
                Q_ASSERT(vertexBetween(node, csNode) == 0); // The two color spaces should not be connected yet
                Vertex* v12 = createVertex(csNode, node);
                v12->factoryFromSrc = csf->createICCColorConversionTransformationFactory( node->modelId, node->depthId);
                Q_ASSERT( v12->factoryFromSrc );
                // Create the vertex from 2 to 1
                Vertex* v21 = createVertex(node, csNode);
                v21->factoryFromSrc = node->colorSpaceFactory->createICCColorConversionTransformationFactory( csNode->modelId, csNode->depthId);
                Q_ASSERT( v21->factoryFromSrc );
            }
        }
        // ICC color space can be converted among the same color space to a different profile, hence the need to a vertex on self
        Vertex* vSelfToSelf = createVertex(csNode, csNode);
        vSelfToSelf->factoryFromSrc = csf->createICCColorConversionTransformationFactory(csNode->modelId, csNode->depthId);
        Q_ASSERT( vSelfToSelf->factoryFromSrc );
    }
    // Construct a link for "custom" transformation
    QList<KoColorConversionTransformationFactory*> cctfs = csf->colorConversionLinks();
    foreach(KoColorConversionTransformationFactory* cctf, cctfs)
    {
        Node* srcNode = nodeFor(cctf->srcColorModelId(), cctf->srcColorDepthId());
        Q_ASSERT(srcNode);
        Node* dstNode = nodeFor(cctf->dstColorModelId(), cctf->dstColorDepthId());
        Q_ASSERT(dstNode);
        Q_ASSERT(srcNode == csNode or dstNode == csNode);
        // Check if the two nodes are allready connected
        Vertex* v = vertexBetween(srcNode, dstNode);
        // If the vertex doesn't allready exist, then create it
        if(not v)
        {
            v = createVertex(srcNode, dstNode);
        }
        Q_ASSERT(v);
        if(dstNode == csNode)
        {
            Q_ASSERT(v->factoryFromDst == 0);
            v->factoryFromDst = cctf;
        } else
        {
            Q_ASSERT(v->factoryFromSrc == 0);
            v->factoryFromSrc = cctf;
        }
    }
}

KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(QString _colorModelId, QString _colorDepthId)
{
    return nodeFor(NodeKey(_colorModelId, _colorDepthId));
}

KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const KoColorConversionSystem::NodeKey& key)
{
    if(!d->graph.contains(key))
    {
        Node* n = new Node;
        n->modelId = key.modelId;
        n->depthId = key.depthId;
        d->graph[key] = n;
        return n;
    }
    return d->graph.value(key);
}


KoColorConversionTransformation* KoColorConversionSystem::createColorConverter(const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent ) const
{
    return 0;
}

KoColorConversionSystem::Vertex* KoColorConversionSystem::vertexBetween(KoColorConversionSystem::Node* srcNode, KoColorConversionSystem::Node* dstNode)
{
    foreach(Vertex* oV, srcNode->outputVertexes)
    {
        if(oV->dstNode == dstNode)
        {
            return oV;
        }
    }
    return 0;
}

KoColorConversionSystem::Vertex* KoColorConversionSystem::createVertex(Node* srcNode, Node* dstNode)
{
    Vertex* v = new Vertex(srcNode, dstNode);
    srcNode->outputVertexes.append( v );
    d->vertexes.append( v );
    return v;
}

// -- Graph visualisation functions --

QString KoColorConversionSystem::toDot() const
{
    QString dot = "digraph CCS {\n";
    foreach(Vertex* oV, d->vertexes)
    {
        dot += QString("%1 -> %2\n").arg(oV->srcNode->colorSpaceFactory->id()).arg(oV->dstNode->colorSpaceFactory->id());
    }
    dot += "}\n";
    return dot;
}
