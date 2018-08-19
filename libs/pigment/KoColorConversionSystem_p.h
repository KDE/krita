/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef KOCOLORCONVERSIONSYSTEM_P_H
#define KOCOLORCONVERSIONSYSTEM_P_H

#include "DebugPigment.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorModelStandardIds.h"
#include "KoColorConversionTransformationFactory.h"
#include "KoColorSpaceEngine.h"

#include <QList>

struct KoColorConversionSystem::Node {

    Node()
        : isHdr(false)
        , isInitialized(false)
        , referenceDepth(0)
        , isGray(false)
        , crossingCost(1)
        , colorSpaceFactory(0)
        , isEngine(false)
        , engine(0) {}

    void init(const KoColorSpaceFactory* _colorSpaceFactory) {
        dbgPigment << "Initialise " << modelId << " " << depthId << " " << profileName;

        if (isInitialized) {
            dbgPigment << "Re-initializing node. Old factory" << colorSpaceFactory << "new factory" << _colorSpaceFactory;
        }
        isInitialized = true;

        if (_colorSpaceFactory) {
            isHdr = _colorSpaceFactory->isHdr();
            colorSpaceFactory = _colorSpaceFactory;
            referenceDepth = _colorSpaceFactory->referenceDepth();
            isGray = (_colorSpaceFactory->colorModelId() == GrayAColorModelID
                      || _colorSpaceFactory->colorModelId() == GrayColorModelID
                      || _colorSpaceFactory->colorModelId() == AlphaColorModelID);
        }
    }

    void init(const KoColorSpaceEngine* _engine) {
        Q_ASSERT(!isInitialized);
        isEngine = true;
        isInitialized = true;
        isHdr = true;
        engine = _engine;
    }

    QString id() const {
        return modelId + " " + depthId + " " + profileName;
    }

    QString modelId;
    QString depthId;
    QString profileName;
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
Q_DECLARE_TYPEINFO(KoColorConversionSystem::Node, Q_MOVABLE_TYPE);

struct KoColorConversionSystem::Vertex {

    Vertex(Node* _srcNode, Node* _dstNode)
        : srcNode(_srcNode)
        , dstNode(_dstNode)
        , factoryFromSrc(0)
        , factoryFromDst(0) {
    }

    ~Vertex() {
        if (factoryFromSrc == factoryFromDst) {
            delete factoryFromSrc;
        } else {
            delete factoryFromSrc;
            delete factoryFromDst;
        }
    }

    void setFactoryFromSrc(KoColorConversionTransformationFactory* factory) {
        factoryFromSrc = factory;
        initParameter(factoryFromSrc);
    }

    void setFactoryFromDst(KoColorConversionTransformationFactory* factory) {
        factoryFromDst = factory;
        if (!factoryFromSrc) initParameter(factoryFromDst);
    }

    void initParameter(KoColorConversionTransformationFactory* transfo) {
        conserveColorInformation = transfo->conserveColorInformation();
        conserveDynamicRange = transfo->conserveDynamicRange();
    }

    KoColorConversionTransformationFactory* factory() {
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

    NodeKey(const QString &_modelId, const QString &_depthId, const QString &_profileName)
        : modelId(_modelId)
        , depthId(_depthId)
        , profileName(_profileName) {}

    bool operator==(const KoColorConversionSystem::NodeKey& rhs) const {
        return modelId == rhs.modelId && depthId == rhs.depthId && profileName == rhs.profileName;
    }

    QString modelId;
    QString depthId;
    QString profileName;
};
Q_DECLARE_TYPEINFO(KoColorConversionSystem::NodeKey, Q_MOVABLE_TYPE);

struct KoColorConversionSystem::Path {

    Path()
        : respectColorCorrectness(true)
        , referenceDepth(0)
        , keepDynamicRange(true)
        , isGood(false)
        , cost(0) {}

    Node* startNode() {
        return vertexes.size() > 0 ?
                    (vertexes.first())->srcNode
                  : 0;
    }

    bool operator==(const Path &other) const {
        return other.vertexes == vertexes;
    }


    const Node* startNode() const {
        return vertexes.size() > 0 ?
                    (vertexes.first())->srcNode
                  : 0;
    }

    Node* endNode() {
        return vertexes.size() > 0 ?
                    (vertexes.last())->dstNode
                  : 0;
    }

    const Node* endNode() const {
        return vertexes.size() > 0 ?
                    (vertexes.last())->dstNode
                  : 0;
    }

    bool isEmpty() const {
        return vertexes.isEmpty();
    }

    void appendVertex(Vertex* v) {
        if (vertexes.empty()) {
            referenceDepth = v->srcNode->referenceDepth;
        }
        vertexes.append(v);
        if (!v->conserveColorInformation) respectColorCorrectness = false;
        if (!v->conserveDynamicRange) keepDynamicRange = false;
        referenceDepth = qMin(referenceDepth, v->dstNode->referenceDepth);
        cost += v->dstNode->crossingCost;
    }

    // Compress path to hide the Engine node and correctly select the factory
    typedef QPair<Node*, const KoColorConversionTransformationAbstractFactory* > node2factory;
    QList< node2factory > compressedPath() const {
        QList< node2factory > nodes;
        nodes.push_back(node2factory(vertexes.first()->srcNode , vertexes.first()->factory()));
        const KoColorConversionTransformationAbstractFactory* previousFactory = 0;
        Q_FOREACH (Vertex* vertex, vertexes) { // Unless the node is the icc node, add it to the path
            Node* n = vertex->dstNode;
            if (n->isEngine) {
                previousFactory = n->engine;
            } else {
                nodes.push_back(
                            node2factory(n,
                                         previousFactory ? previousFactory : vertex->factory()));
                previousFactory = 0;
            }
        }
        return nodes;
    }

    int length() const {
        return vertexes.size();
    }

    bool contains(Node* n) const {
        Q_FOREACH (Vertex* v, vertexes) {
            if (v->srcNode == n || v->dstNode == n) {
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
Q_DECLARE_TYPEINFO(KoColorConversionSystem::Path, Q_MOVABLE_TYPE);


inline QDebug operator<<(QDebug dbg, const KoColorConversionSystem::Path &path)
{
    bool havePrintedFirst = false;

    Q_FOREACH (const KoColorConversionSystem::Vertex *v, path.vertexes) {
        if (!havePrintedFirst) {
            dbg.nospace() << v->srcNode->id();
            havePrintedFirst = true;
        }

        dbg.nospace() << "->" << v->dstNode->id();
    }

    return dbg.space();
}


typedef QHash<KoColorConversionSystem::Node*, KoColorConversionSystem::Path > Node2PathHash;


uint qHash(const KoColorConversionSystem::NodeKey &key)
{
    return qHash(key.modelId) + qHash(key.depthId);
}

struct Q_DECL_HIDDEN KoColorConversionSystem::Private {

    Private(RegistryInterface *_registryInterface) : registryInterface(_registryInterface) {}

    QHash<NodeKey, Node*> graph;
    QList<Vertex*> vertexes;
    RegistryInterface *registryInterface;
};

#define CHECK_ONE_AND_NOT_THE_OTHER(name) \
    if(path1. name && !path2. name) \
{ \
    return true; \
    } \
    if(!path1. name && path2. name) \
{ \
    return false; \
    }

struct PathQualityChecker {

    PathQualityChecker(int _referenceDepth, bool _ignoreHdr, bool _ignoreColorCorrectness)
        : referenceDepth(_referenceDepth)
        , ignoreHdr(_ignoreHdr)
        , ignoreColorCorrectness(_ignoreColorCorrectness)
    {}

    /// @return true if the path maximize all the criterions (except length)
    inline bool isGoodPath(const KoColorConversionSystem::Path & path) const {

        return (path.respectColorCorrectness || ignoreColorCorrectness) &&
                (path.referenceDepth >= referenceDepth) &&
                (path.keepDynamicRange || ignoreHdr);
    }

    /**
     * Compare two paths.
     */
    inline bool lessWorseThan(const KoColorConversionSystem::Path &path1, const KoColorConversionSystem::Path &path2) const {
        // There is no point in comparing two paths which doesn't start from the same node or doesn't end at the same node
        if (!ignoreHdr) {
            CHECK_ONE_AND_NOT_THE_OTHER(keepDynamicRange)
        }
        if (!ignoreColorCorrectness) {
            CHECK_ONE_AND_NOT_THE_OTHER(respectColorCorrectness)
        }
        if (path1.referenceDepth == path2.referenceDepth) {
            return path1.cost < path2.cost; // if they have the same cost, well anyway you have to choose one, and there is no point in keeping one and not the other
        }
        return path1.referenceDepth > path2.referenceDepth;
    }
    int referenceDepth;
    bool ignoreHdr;
    bool ignoreColorCorrectness;
};

#undef CHECK_ONE_AND_NOT_THE_OTHER

#endif
