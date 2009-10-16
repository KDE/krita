/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
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

#include <QList>

struct KoColorConversionSystem::Node {
    
    Node() 
        : isIcc(false)
        , isHdr(false)
        , isInitialized(false)
        , referenceDepth(0)
        , isGray(false)
        , crossingCost(1)
        , colorSpaceFactory(0)
        , isEngine(false)
        , engine(0) {}
    
    void init( const KoColorSpaceFactory* _colorSpaceFactory)
    {
        dbgPigment << "Initialise " << modelId << " " << depthId << " " << profileName;
        Q_ASSERT(not isInitialized);
        isInitialized = true;
        
        if(_colorSpaceFactory)
        {
            isIcc = _colorSpaceFactory->colorSpaceEngine() == "icc";
            isHdr = _colorSpaceFactory->isHdr();
            colorSpaceFactory = _colorSpaceFactory;
            referenceDepth = _colorSpaceFactory->referenceDepth();
            isGray = ( _colorSpaceFactory->colorModelId() == GrayAColorModelID
                       or _colorSpaceFactory->colorModelId() == GrayColorModelID );
        }
    }
    
    void init( const KoColorSpaceEngine* _engine)
    {
        Q_ASSERT(not isInitialized);
        isEngine = true;
        isInitialized = true;
        engine = _engine;
    }
    
    QString id() const {
        return modelId + " " + depthId + " " + profileName;
    }
    
    QString modelId;
    QString depthId;
    QString profileName;
    bool isIcc;
    bool isHdr;
    bool isInitialized;
    int referenceDepth;
    QList<Vertex*> outputVertexes;
    bool isGray;
    int crossingCost;
    const KoColorSpaceFactory* colorSpaceFactory;
    bool isEngine;
    const KoColorSpaceEngine* engine;
};

struct KoColorConversionSystem::Vertex {
    
    Vertex(Node* _srcNode, Node* _dstNode) 
        : srcNode(_srcNode)
        , dstNode(_dstNode)
        , factoryFromSrc(0)
        , factoryFromDst(0)
    {
    }
    
    ~Vertex()
    {
        if (factoryFromSrc == factoryFromDst) {
            delete factoryFromSrc;
        }
        else {
            delete factoryFromSrc;
            delete factoryFromDst;
        }
    }
    
    void setFactoryFromSrc(KoColorConversionTransformationFactory* factory)
    {
        factoryFromSrc = factory;
        initParameter(factoryFromSrc);
    }
    
    void setFactoryFromDst(KoColorConversionTransformationFactory* factory)
    {
        factoryFromDst = factory;
        if (!factoryFromSrc) initParameter(factoryFromDst);
    }
    
    void initParameter(KoColorConversionTransformationFactory* transfo)
    {
        conserveColorInformation = transfo->conserveColorInformation();
        conserveDynamicRange = transfo->conserveDynamicRange();
    }
    
    KoColorConversionTransformationFactory* factory()
    {
        if (factoryFromSrc) return factoryFromSrc;
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

    NodeKey(QString _modelId, QString _depthId, QString _profileName)
        : modelId(_modelId)
        , depthId(_depthId)
        , profileName(_profileName)
    {}

    bool operator==(const KoColorConversionSystem::NodeKey& rhs) const
    {
        return modelId == rhs.modelId and depthId == rhs.depthId and profileName == rhs.profileName;
    }

    QString modelId;
    QString depthId;
    QString profileName;
};

struct KoColorConversionSystem::Path {

    Path()
        : respectColorCorrectness(true)
        , referenceDepth(0)
        , keepDynamicRange(true)
        , isGood(false)
        , cost(0)
    {}

    Node* startNode() {
        return (vertexes.first())->srcNode;
    }

    const Node* startNode() const {
        return (vertexes.first())->srcNode;
    }

    Node* endNode() {
        return (vertexes.last())->dstNode;
    }

    const Node* endNode() const {
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
        cost += v->dstNode->crossingCost;
    }

    // Compress path to hide the Engine node and correctly select the factory
    typedef QPair<Node*, const KoColorConversionTransformationAbstractFactory* > node2factory;
    QList< node2factory > compressedPath() const
    {
        QList< node2factory > nodes;
        nodes.push_back( node2factory( vertexes.first()->srcNode , vertexes.first()->factory() ) );
        const KoColorConversionTransformationAbstractFactory* previousFactory = 0;
        foreach( Vertex* vertex, vertexes)
        { // Unless the node is the icc node, add it to the path
            Node* n = vertex->dstNode;
            if( n->isEngine  )
            {
                previousFactory = n->engine;
            } else {
                nodes.push_back(
                        node2factory( n,
                                      previousFactory ? previousFactory : vertex->factory() ) );
                previousFactory = 0;
            }
        }
        return nodes;
    }

    int length() const
    {
        return vertexes.size();
    }

    bool contains(Node* n) const
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
    int cost;
};

class Node2PathHash : public QHash<KoColorConversionSystem::Node*, KoColorConversionSystem::Path*>
{
public:
    ~Node2PathHash() { qDeleteAll(*this); }
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

#define CHECK_ONE_AND_NOT_THE_OTHER(name) \
if(path1-> name and not path2-> name) \
{ \
  return true; \
} \
if(not path1-> name and path2-> name) \
{ \
  return false; \
}

struct PathQualityChecker {
    PathQualityChecker(int _referenceDepth, bool _ignoreHdr, bool _ignoreColorCorrectness) : referenceDepth(_referenceDepth), ignoreHdr(_ignoreHdr), ignoreColorCorrectness(_ignoreColorCorrectness) {}
    /// @return true if the path maximize all the criterions (except length)
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
            return path1->cost < path2->cost; // if they have the same cost, well anyway you have to choose one, and there is no point in keeping one and not the other
        }
        return path1->referenceDepth > path2->referenceDepth;
    }
    int referenceDepth;
    bool ignoreHdr;
    bool ignoreColorCorrectness;
};

#undef CHECK_ONE_AND_NOT_THE_OTHER
