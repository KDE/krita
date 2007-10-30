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

#include "KoColorConversionAlphaTransformation.h"
#include "KoColorConversionTransformation.h"
#include "KoColorConversionTransformationFactory.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorModelStandardIds.h"
#include "KoCopyColorConversionTransformation.h"

class KoMultipleColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KoMultipleColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent = IntentPerceptual) : KoColorConversionTransformation(srcCs, dstCs, renderingIntent), m_maxPixelSize(qMax(srcCs->pixelSize(), dstCs->pixelSize()))
        {
            
        }
        ~KoMultipleColorConversionTransformation()
        {
            foreach(KoColorConversionTransformation* transfo, m_transfos)
            {
                delete transfo;
            }
        }
        void appendTransfo(KoColorConversionTransformation* transfo)
        {
            m_transfos.append( transfo );
            m_maxPixelSize = qMax(m_maxPixelSize, transfo->srcColorSpace()->pixelSize());
            m_maxPixelSize = qMax(m_maxPixelSize, transfo->dstColorSpace()->pixelSize());
        }
        virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
        {
            Q_ASSERT(m_transfos.size() > 1); // Be sure to have a more than one transformation
            quint8 *buff1 = new quint8[m_maxPixelSize*nPixels];
            quint8 *buff2 = 0;
            if(m_transfos.size() > 2)
            {
                buff2 = new quint8[m_maxPixelSize*nPixels]; // a second buffer is needed
            }
            m_transfos.first()->transform( src, buff1, nPixels);
            int lastIndex = m_transfos.size() - 2;
            for( int i = 1; i <= lastIndex; i++)
            {
                m_transfos[i]->transform( buff1, buff2, nPixels);
                quint8* tmp = buff1;
                buff1 = buff2;
                buff2 = tmp;
            }
            m_transfos.last()->transform( buff1, dst, nPixels);
            delete buff2;
            delete buff1;
        }
    private:
        QList<KoColorConversionTransformation*> m_transfos;
        quint32 m_maxPixelSize;
};

struct KoColorConversionSystem::Node {
    Node() : isIcc(false), isHdr(false), isInitialized(false), referenceDepth(0), isGray(false), canBeCrossed(true), colorSpaceFactory(0) {}
    void init( const KoColorSpaceFactory* _colorSpaceFactory)
    {
        Q_ASSERT(not isInitialized);
        isInitialized = true;
        
        if(_colorSpaceFactory)
        {
            isIcc = _colorSpaceFactory->isIcc();
            isHdr = _colorSpaceFactory->isHdr();
            colorSpaceFactory = _colorSpaceFactory;
            referenceDepth = _colorSpaceFactory->referenceDepth();
            isGray = ( _colorSpaceFactory->colorModelId() == GrayAColorModelID
                    or _colorSpaceFactory->colorModelId() == GrayColorModelID );
        }
    }
    QString id() const {
        return modelId + " " + depthId;
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
    bool canBeCrossed;
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
        if( not factoryFromSrc) initParameter(factoryFromDst);
    }
    void initParameter(KoColorConversionTransformationFactory* transfo)
    {
        conserveColorInformation = transfo->conserveColorInformation();
        conserveDynamicRange = transfo->conserveDynamicRange();
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
    Path() : respectColorCorrectness(true), referenceDepth(0), keepDynamicRange(true), isGood(false)
    {}
    Node* startNode() {
        return (vertexes.first())->srcNode;
    }
    Node* endNode() {
        return (vertexes.last())->dstNode;
    }
    void appendVertex(Vertex* v) {
        if(vertexes.empty())
        {
            referenceDepth = v->srcNode->referenceDepth;
        }
        vertexes.append(v);
        if(not v->conserveColorInformation) respectColorCorrectness = false;
        if(not v->conserveDynamicRange) keepDynamicRange = false;
        referenceDepth = qMin( referenceDepth, v->dstNode->referenceDepth);
    }
    int length() {
        return vertexes.size();
    }
    bool contains(Node* n)
    {
        foreach(Vertex* v, vertexes)
        {
            if(v->srcNode == n or v->dstNode == n)
            {
                return true;
            }
        }
        return false;
    }
    QList<Vertex*> vertexes;
    bool respectColorCorrectness;
    int referenceDepth;
    bool keepDynamicRange;
    bool isGood;
};

uint qHash(const KoColorConversionSystem::NodeKey &key)
{
    return qHash(key.modelId) + qHash(key.depthId);
}

struct KoColorConversionSystem::Private {
    QHash<NodeKey, Node*> graph;
    QList<Vertex*> vertexes;
    Node* alphaNode;
};


KoColorConversionSystem::KoColorConversionSystem() : d(new Private)
{
    // Create the Alpha 8bit
    d->alphaNode = nodeFor(AlphaColorModelID.id(), Integer8BitsColorDepthID.id());
    d->alphaNode->canBeCrossed = false;
    d->alphaNode->init(0);
    d->alphaNode->isGray = true; // <- FIXME: it's a little bit hacky as alpha doesn't really have color information
    Vertex* v = createVertex(d->alphaNode, d->alphaNode);
    v->setFactoryFromSrc( new KoCopyColorConversionTransformationFactory(AlphaColorModelID.id(), Integer8BitsColorDepthID.id()));
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
    // Alpha connection
    Q_ASSERT(vertexBetween(d->alphaNode, csNode) == 0); // The two color spaces should not be connected yet
    Vertex* vFromAlpha = createVertex(d->alphaNode, csNode);
    vFromAlpha->setFactoryFromSrc( new KoColorConversionFromAlphaTransformationFactory( modelId, depthId ) );
    Q_ASSERT(vertexBetween(csNode, d->alphaNode) == 0); // The two color spaces should not be connected yet
    Vertex* vToAlpha = createVertex(csNode, d->alphaNode);
    vToAlpha->setFactoryFromDst( new KoColorConversionToAlphaTransformationFactory( modelId, depthId ) );
    // ICC Connection
    if(csNode->isIcc)
    { // Construct a link between this color space and all other ICC color space
        kDebug(31000) << csf->id() << " is an ICC color space, connecting to others";
        QList<Node*> nodes = d->graph.values();
        foreach(Node* node, nodes)
        {
            if(node->isIcc and node->isInitialized and node != csNode)
            {
                // Create the vertex from 1 to 2
                Q_ASSERT(vertexBetween(csNode, node) == 0); // The two color spaces should not be connected yet
                Vertex* v12 = createVertex(csNode, node);
                v12->setFactoryFromSrc( csf->createICCColorConversionTransformationFactory( node->modelId, node->depthId) );
                Q_ASSERT( v12->factory() );
                // Create the vertex from 2 to 1
                Q_ASSERT(vertexBetween(node, csNode) == 0); // The two color spaces should not be connected yet
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
    kDebug(31000) << csf->id() << " has " << cctfs.size() << " direct connections";
    foreach(KoColorConversionTransformationFactory* cctf, cctfs)
    {
        Node* srcNode = nodeFor(cctf->srcColorModelId(), cctf->srcColorDepthId());
        Q_ASSERT(srcNode);
        Node* dstNode = nodeFor(cctf->dstColorModelId(), cctf->dstColorDepthId());
        Q_ASSERT(dstNode);
        kDebug(31000) << "Connecting " << srcNode->id() << " to " << dstNode->id();
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
            v->setFactoryFromDst(cctf);
        } else
        {
            v->setFactoryFromSrc(cctf);
        }
    }
    // Check if there is a path to convert self into self
    Vertex* v = vertexBetween(csNode, csNode);
    if( not v)
    {
        v = createVertex(csNode, csNode);
        kDebug(31000) << "No self to self color conversion, add the copy one";
        v->setFactoryFromSrc( new KoCopyColorConversionTransformationFactory(modelId, depthId));
    }
}

KoColorSpace* KoColorConversionSystem::defaultColorSpaceForNode(const Node* node) const
{
    return KoColorSpaceRegistry::instance()->colorSpace( KoColorSpaceRegistry::instance()->colorSpaceId( node->modelId, node->depthId ), 0 );
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
    if(not d->graph.contains(key))
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
    Path* path = findBestPath( nodeFor( srcColorSpace->colorModelId().id(), srcColorSpace->colorDepthId().id() ), nodeFor( dstColorSpace->colorModelId().id(), dstColorSpace->colorDepthId().id() ) );
    Q_ASSERT(path);
    KoColorConversionTransformation* transfo = 0;
    if(path->length() == 1)
    { // Direct connection
        transfo = path->vertexes.first()->factory()->createColorTransformation( srcColorSpace, dstColorSpace, renderingIntent );
    } else {
        KoMultipleColorConversionTransformation* mccTransfo = new KoMultipleColorConversionTransformation(srcColorSpace, dstColorSpace, renderingIntent);
        transfo = mccTransfo;
        KoColorSpace* intermCS = defaultColorSpaceForNode( path->vertexes.first()->dstNode );
        mccTransfo->appendTransfo( path->vertexes.first()->factory()->createColorTransformation(srcColorSpace, intermCS, renderingIntent) );
        
        kDebug(31000) << path->vertexes.first()->srcNode->id() << " to " << path->vertexes.first()->dstNode->id();
        for(int i = 1; i < path->length() - 1; i++)
        {
            Vertex* v = path->vertexes[i];
            kDebug(31000) << v->srcNode->id() << " to " << v->dstNode->id();
            KoColorSpace* intermCS2 = defaultColorSpaceForNode( v->dstNode );
            mccTransfo->appendTransfo( v->factory()->createColorTransformation(intermCS, intermCS2, renderingIntent) );
            intermCS = intermCS2;
        }
        kDebug(31000) << path->vertexes.last()->srcNode->id() << " to " << path->vertexes.last()->dstNode->id();
        mccTransfo->appendTransfo( path->vertexes.last()->factory()->createColorTransformation(intermCS, dstColorSpace, renderingIntent) );
        
    }
    delete path;
    Q_ASSERT(transfo);
    return transfo;
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
    return QString("  \"%1\" -> \"%2\" %3\n").arg(v->srcNode->id()).arg(v->dstNode->id()).arg(options);
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

bool KoColorConversionSystem::existsPath( QString srcModelId, QString srcDepthId, QString dstModelId, QString dstDepthId ) const
{
    Path* path = findBestPath( nodeFor( srcModelId, srcDepthId ), nodeFor( dstModelId, dstDepthId ) );
    bool exist = path;
    delete path;
    return exist;
}

bool KoColorConversionSystem::existsGoodPath( QString srcModelId, QString srcDepthId, QString dstModelId, QString dstDepthId ) const
{
    Path* path = findBestPath( nodeFor( srcModelId, srcDepthId ), nodeFor( dstModelId, dstDepthId ) );
    bool existAndGood = path and path->isGood;
    delete path;
    return existAndGood;
}


QString KoColorConversionSystem::bestPathToDot(QString srcModelId, QString srcDepthId, QString dstModelId, QString dstDepthId) const
{
    const Node* srcNode = nodeFor(srcModelId, srcDepthId);
    const Node* dstNode = nodeFor(dstModelId, dstDepthId);
    Path* p = findBestPath( srcNode, dstNode);
    Q_ASSERT(p);
    QString dot = "digraph CCS {\n";
    dot += QString("  \"%1\" [color=red]\n").arg(srcNode->id());
    dot += QString("  \"%1\" [color=red]\n").arg(dstNode->id());
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
    if(path1-> name and not path2-> name) \
    { \
        return true; \
    } \
    if(not path1-> name and path2-> name) \
    { \
        return false; \
    }

template<bool ignoreHdr, bool ignoreColorCorrectness>
struct PathQualityChecker {
    PathQualityChecker(int _referenceDepth) : referenceDepth(_referenceDepth) {}
    /// @return true if the path maximize all the criterions (except lenght)
    inline bool isGoodPath(KoColorConversionSystem::Path* path)
    {
        return ( path->respectColorCorrectness or ignoreColorCorrectness ) and
               ( path->referenceDepth >= referenceDepth) and
               ( path->keepDynamicRange or ignoreHdr );
    }
    /**
     * Compare two pathes.
     */
    inline bool lessWorseThan(KoColorConversionSystem::Path* path1, KoColorConversionSystem::Path* path2)
    {
         // There is no point in comparing two pathes which doesn't start from the same node or doesn't end at the same node
        Q_ASSERT(path1->startNode() == path2->startNode());
        Q_ASSERT(path1->endNode() == path2->endNode());
        if(not ignoreHdr)
        {
            CHECK_ONE_AND_NOT_THE_OTHER(keepDynamicRange)
        }
        if(not ignoreColorCorrectness)
        {
            CHECK_ONE_AND_NOT_THE_OTHER(respectColorCorrectness)
        }
        if( path1->referenceDepth == path2->referenceDepth)
        {
            return path1->length() < path2->length(); // if they have the same length, well anyway you have to choose one, and there is no point in keeping one and not the other
        }
        return path1->referenceDepth > path2->referenceDepth;
    }
    int referenceDepth;
};

void KoColorConversionSystem::deletePathes( QList<KoColorConversionSystem::Path*> pathes) const
{
    foreach(Path* path, pathes)
    {
        delete path;
    }
}

class Node2PathHash : public QHash<KoColorConversionSystem::Node*, KoColorConversionSystem::Path*>
{
public:
    ~Node2PathHash() { qDeleteAll(*this); }
};

template<bool ignoreHdr, bool ignoreColorCorrectness>
inline KoColorConversionSystem::Path* KoColorConversionSystem::findBestPathImpl2( const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
{
    PathQualityChecker<ignoreHdr, ignoreColorCorrectness> pQC( qMin(srcNode->referenceDepth, dstNode->referenceDepth ) );
    Node2PathHash node2path; // current best path to reach a given node
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
            p->isGood = true;
            return p;
        } else if( endNode->canBeCrossed)
        {
            Q_ASSERT(not node2path.contains( endNode )); // That would be a total fuck up if there are two vertexes between two nodes
            node2path[ endNode ] = new Path( *p );
            currentPathes.append( p );
        }
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
                if( not p->contains( v->dstNode ) )
                {
                    Path* newP = new Path(*p);
                    newP->appendVertex( v );
                    Node* newEndNode = newP->endNode();
                    if( newEndNode == dstNode)
                    {
                        if( pQC.isGoodPath(newP) )
                        { // Victory
                            deletePathes(currentPathes); // clean up
                            newP->isGood = true;
                            return newP;
                        } else if( not lessWorsePath )
                        {
                            lessWorsePath = newP;
                        } else if( pQC.lessWorseThan( newP, lessWorsePath)  ) {
                            delete lessWorsePath;
                            lessWorsePath = newP;
                        } else {
                            delete newP;
                        }
                    } else if( endNode->canBeCrossed) {
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
        kWarning(31000) << "No good path from " << srcNode->id() << " to " << dstNode->id() << " found !";
        return lessWorsePath;
    } 
    kError(31000) << "No path from " << srcNode->id() << " to " << dstNode->id() << " found !";
    return 0;
}

template<bool ignoreHdr>
inline KoColorConversionSystem::Path* KoColorConversionSystem::findBestPathImpl( const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
{
    if(srcNode->isGray or dstNode->isGray)
    {
        return findBestPathImpl2<ignoreHdr, true>(srcNode, dstNode);
    } else {
        return findBestPathImpl2<ignoreHdr, false>(srcNode, dstNode);
    }
}

KoColorConversionSystem::Path* KoColorConversionSystem::findBestPath( const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
{
//     kDebug(31000) << "Find best path between " << srcNode->id() << " and " << dstNode->id();
    if(srcNode->isHdr and dstNode->isHdr)
    {
        return findBestPathImpl<false>(srcNode, dstNode);
    } else {
        return findBestPathImpl<true>(srcNode, dstNode);
    }
}
