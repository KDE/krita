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
    bool isGray;
    bool canBeCrossed; ///< indicates wether this node can be use in the middle of a path
    const KoColorSpaceFactory* colorSpaceFactory;
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
    }
    int length() const {
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
    bool ignoreHdr;
    bool ignoreColorCorrectness;
};

#undef CHECK_ONE_AND_NOT_THE_OTHER
