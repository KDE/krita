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
#include "KoColorModelStandardIds.h"

struct KoColorConversionSystem::Node {
    Node() : isInitialized(false), colorSpaceFactory(0) {}
    void init( const KoColorSpaceFactory* _colorSpaceFactory)
    {
        Q_ASSERT(!isInitialized);
        isInitialized = true;
        isIcc = _colorSpaceFactory->isIcc();
        isHdr = _colorSpaceFactory->isHdr();
        colorSpaceFactory = _colorSpaceFactory;
        referenceDepth = _colorSpaceFactory->referenceDepth();
        isGray = ( _colorSpaceFactory->colorModelId() == GrayAColorModelID
                || _colorSpaceFactory->colorModelId() == GrayColorModelID );
    }
    QString id() const {
        return colorSpaceFactory->id();
    }
    QString modelId;
    QString depthId;
    bool isIcc;
    bool isHdr;
    bool isInitialized;
    int referenceDepth;
    QList<Vertex*> outputVertexes;
    const KoColorSpaceFactory* colorSpaceFactory;
    bool isGray;
};

struct KoColorConversionSystem::Vertex {
    Vertex(Node* _srcNode, Node* _dstNode) : srcNode(_srcNode), dstNode(_dstNode), factoryFromSrc(0), factoryFromDst(0)
    {
    }
    ~Vertex()
    {
        delete factoryFromSrc;
        delete factoryFromDst;
    }
    void setFactoryFromSrc(KoColorConversionTransformationFactory* factory)
    {
        factoryFromSrc = factory;
        initParameter(factoryFromSrc);
    }
    void setFactoryFromDst(KoColorConversionTransformationFactory* factory)
    {
        factoryFromDst = factory;
        if(!factoryFromSrc) initParameter(factoryFromDst);
    }
    void initParameter(KoColorConversionTransformationFactory* transfo)
    {
        conserveColorInformation = transfo->conserveColorInformation();
        conserveDynamicRange = transfo->conserveDynamicRange();
        depthDecrease = transfo->depthDecrease();
    }
    KoColorConversionTransformationFactory* factory()
    {
        if(factoryFromSrc) return factoryFromSrc;
        return factoryFromDst;
    }

    Node* srcNode;
    Node* dstNode;

    bool conserveColorInformation;
    bool conserveDynamicRange;
    int depthDecrease;
    private:
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

struct KoColorConversionSystem::Path {
    Path() : respectColorCorrectness(true), bitDepthDecrease(0), keepDynamicRange(true)
    {}
    Node* startNode() {
        return (vertexes.first())->srcNode;
    }
    Node* endNode() {
        return (vertexes.last())->dstNode;
    }
    void appendVertex(Vertex* v) {
        vertexes.append(v);
        if(!v->conserveColorInformation) respectColorCorrectness = false;
        if(!v->conserveDynamicRange) keepDynamicRange = false;
        bitDepthDecrease += v->depthDecrease;
    }
    int length() {
        return vertexes.size();
    }
    bool contains(Node* n)
    {
        foreach(Vertex* v, vertexes)
        {
            if(v->srcNode == n || v->dstNode == n)
            {
                return true;
            }
        }
        return false;
    }
    QList<Vertex*> vertexes;
    bool respectColorCorrectness;
    int bitDepthDecrease;
    bool keepDynamicRange;
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
    kDebug(31000) << "Inserting color space " << csf->name() << " (" << csf->id() << ") Model: " << csf->colorModelId() << " Depth: " << csf->colorDepthId() << " into the CCS";
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
            if(node->isIcc && node->isInitialized && node != csNode)
            {
                // Create the vertex from 1 to 2
                Q_ASSERT(vertexBetween(node, csNode) == 0); // The two color spaces should not be connected yet
                Vertex* v12 = createVertex(csNode, node);
                v12->setFactoryFromSrc( csf->createICCColorConversionTransformationFactory( node->modelId, node->depthId) );
                Q_ASSERT( v12->factory() );
                // Create the vertex from 2 to 1
                Vertex* v21 = createVertex(node, csNode);
                v21->setFactoryFromSrc( node->colorSpaceFactory->createICCColorConversionTransformationFactory( csNode->modelId, csNode->depthId) );
                Q_ASSERT( v21->factory() );
            }
        }
        // ICC color space can be converted among the same color space to a different profile, hence the need to a vertex on self
        Vertex* vSelfToSelf = createVertex(csNode, csNode);
        vSelfToSelf->setFactoryFromSrc( csf->createICCColorConversionTransformationFactory(csNode->modelId, csNode->depthId) );
        Q_ASSERT( vSelfToSelf->factory() );
    }
    // Construct a link for "custom" transformation
    QList<KoColorConversionTransformationFactory*> cctfs = csf->colorConversionLinks();
    foreach(KoColorConversionTransformationFactory* cctf, cctfs)
    {
        Node* srcNode = nodeFor(cctf->srcColorModelId(), cctf->srcColorDepthId());
        Q_ASSERT(srcNode);
        Node* dstNode = nodeFor(cctf->dstColorModelId(), cctf->dstColorDepthId());
        Q_ASSERT(dstNode);
        Q_ASSERT(srcNode == csNode || dstNode == csNode);
        // Check if the two nodes are allready connected
        Vertex* v = vertexBetween(srcNode, dstNode);
        // If the vertex doesn't allready exist, then create it
        if(!v)
        {
            v = createVertex(srcNode, dstNode);
        }
        Q_ASSERT(v);
        if(dstNode == csNode)
        {
            v->setFactoryFromDst(cctf);
        } else
        {
            v->setFactoryFromSrc(cctf);
        }
    }
}

const KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(QString _colorModelId, QString _colorDepthId) const 
{
    return nodeFor(NodeKey(_colorModelId, _colorDepthId));
}

const KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const KoColorConversionSystem::NodeKey& key) const
{
    Q_ASSERT(d->graph.contains(key));
    return d->graph.value(key);
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

QString KoColorConversionSystem::vertexToDot(KoColorConversionSystem::Vertex* v, QString options) const
{
    return QString("  %1 -> %2 %3\n").arg(v->srcNode->colorSpaceFactory->id()).arg(v->dstNode->colorSpaceFactory->id()).arg(options);
}

QString KoColorConversionSystem::toDot() const
{
    QString dot = "digraph CCS {\n";
    foreach(Vertex* oV, d->vertexes)
    {
        dot += vertexToDot(oV, "") ;
    }
    dot += "}\n";
    return dot;
}

QString KoColorConversionSystem::bestPathToDot(QString srcModelId, QString srcDepthId, QString dstModelId, QString dstDepthId) const
{
    const Node* srcNode = nodeFor(srcModelId, srcDepthId);
    const Node* dstNode = nodeFor(dstModelId, dstDepthId);
    Path* p = findBestPath( srcNode, dstNode);
    Q_ASSERT(p);
    QString dot = "digraph CCS {\n";
    dot += QString("  %1 [color=red]\n").arg(srcNode->colorSpaceFactory->id());
    dot += QString("  %1 [color=red]\n").arg(dstNode->colorSpaceFactory->id());
    foreach(Vertex* oV, d->vertexes)
    {
        QString options = "";
        if(p->vertexes.contains( oV))
        {
            options = "[color=red]";
        }
        dot += vertexToDot(oV, options) ;
    }
    dot += "}\n";
    return dot;
}

#define CHECK_ONE_AND_NOT_THE_OTHER(name) \
    if(path1-> name && !path2-> name) \
    { \
        return true; \
    } \
    if(!path1-> name && path2-> name) \
    { \
        return false; \
    }

template<bool ignoreHdr, bool ignoreColorCorrectness>
struct PathQualityChecker {
    PathQualityChecker(int _maxBitDecrease) : maxBitDecrease(_maxBitDecrease) {}
    /// @return true if the path maximize all the criterions (except lenght)
    inline bool isGoodPath(KoColorConversionSystem::Path* path)
    {
        return ( path->respectColorCorrectness || ignoreColorCorrectness ) &&
               ( path->bitDepthDecrease <= maxBitDecrease) &&
               ( path->keepDynamicRange || ignoreHdr );
    }
    /**
     * Compare two pathes.
     */
    inline bool lessWorseThan(KoColorConversionSystem::Path* path1, KoColorConversionSystem::Path* path2)
    {
         // There is no point in comparing two pathes which doesn't start from the same node or doesn't end at the same node
        Q_ASSERT(path1->startNode() == path2->startNode());
        Q_ASSERT(path1->endNode() == path2->endNode());
        if(!ignoreHdr)
        {
            CHECK_ONE_AND_NOT_THE_OTHER(keepDynamicRange)
        }
        if(!ignoreColorCorrectness)
        {
            CHECK_ONE_AND_NOT_THE_OTHER(respectColorCorrectness)
        }
        if( path1->bitDepthDecrease == path2->bitDepthDecrease)
        {
            return path1->length() < path2->length(); // if they have the same length, well anyway you have to choose one, and there is no point in keeping one and not the other
        }
        return path1->bitDepthDecrease < path2->bitDepthDecrease;
    }
    int maxBitDecrease;
};

void KoColorConversionSystem::deletePathes( QList<KoColorConversionSystem::Path*> pathes) const
{
    foreach(Path* path, pathes)
    {
        delete path;
    }
}

template<bool ignoreHdr, bool ignoreColorCorrectness>
inline KoColorConversionSystem::Path* KoColorConversionSystem::findBestPathImpl( const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
{
    PathQualityChecker<ignoreHdr, ignoreColorCorrectness> pQC( qMax(0, dstNode->referenceDepth - srcNode->referenceDepth ) );
    QHash<Node*, Path*> node2path; // current best path to reach a given node
    QList<Path*> currentPathes; // list of all pathes
    // Generate the initial list of pathes
    foreach( Vertex* v, srcNode->outputVertexes)
    {
        Path* p = new Path;
        p->appendVertex(v);
        Node* endNode = p->endNode();
        if( endNode == dstNode)
        {
            Q_ASSERT( pQC.isGoodPath(p)); // <- it's a direct link, it has to be a good path, damn it ! or go fix your color space !
            deletePathes(currentPathes); // clean up
            return p;
        }
        Q_ASSERT(!node2path.contains( endNode )); // That would be a total fuck up if there are two vertexes between two nodes
        node2path[ endNode ] = new Path( *p );
        currentPathes.append( p );
    }
    Path* lessWorsePath = 0;
    // Now loop until a path has been found
    while( currentPathes.size() > 0 )
    {
        foreach(Path* p, currentPathes)
        {
            Node* endNode = p->endNode();
            foreach( Vertex* v, endNode->outputVertexes)
            {
                if(!p->contains( v->dstNode ) )
                {
                    Path* newP = new Path(*p);
                    newP->appendVertex( v );
                    Node* newEndNode = newP->endNode();
                    if( newEndNode == dstNode)
                    {
                        if( pQC.isGoodPath(newP) )
                        { // Victory
                            deletePathes(currentPathes); // clean up
                            return newP;
                        } else if(!lessWorsePath )
                        {
                            lessWorsePath = newP;
                        } else if( pQC.lessWorseThan( newP, lessWorsePath)  ) {
                            delete lessWorsePath;
                            lessWorsePath = newP;
                        } else {
                            delete newP;
                        }
                    } else {
                        if( node2path.contains( newEndNode ) )
                        {
                            Path* p2 = node2path[newEndNode];
                            if( pQC.lessWorseThan( newP, p2 ) )
                            {
                                node2path[ newEndNode ] = new Path(*newP);
                                currentPathes.append( newP );
                                delete p2;
                            } else {
                                delete newP;
                            }
                        } else {
                            node2path[ newEndNode ] = new Path(*newP);
                           currentPathes.append( newP );
                        }
                    }
                }
            }
            currentPathes.removeAll( p );
            delete p;
        }
    }
    if(lessWorsePath)
    {
        kWarning() << "No good path from " << srcNode->colorSpaceFactory->id() << " to " << dstNode->colorSpaceFactory->id() << " found !";
        return lessWorsePath;
    } 
    kError() << "No path from " << srcNode->colorSpaceFactory->id() << " to " << dstNode->colorSpaceFactory->id() << " found !";
    return 0;
}

template<bool ignoreHdr>
inline KoColorConversionSystem::Path* KoColorConversionSystem::findBestPathImpl( const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
{
    if(srcNode->isGray || dstNode->isGray)
    {
        return findBestPathImpl<ignoreHdr, true>(srcNode, dstNode);
    } else {
        return findBestPathImpl<ignoreHdr, false>(srcNode, dstNode);
    }
}

KoColorConversionSystem::Path* KoColorConversionSystem::findBestPath( const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
{
    if(srcNode->isHdr && dstNode->isHdr)
    {
        return findBestPathImpl<false>(srcNode, dstNode);
    } else {
        return findBestPathImpl<true>(srcNode, dstNode);
    }
}
