/*
 *  SPDX-FileCopyrightText: 2007-2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KOCOLORCONVERSIONSYSTEM_P_H
#define KOCOLORCONVERSIONSYSTEM_P_H

#include "DebugPigment.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorModelStandardIds.h"
#include "KoColorConversionTransformationFactory.h"
#include "KoColorSpaceEngine.h"
#include "KoColorConversionSystem.h"
#include <boost/operators.hpp>

#include <QList>

enum NodeCapability {
    None = 0x0,
    HasColor = 0x1,
    HasAlpha = 0x2,
    HasHdr = 0x4
};

Q_DECLARE_FLAGS(NodeCapabilities, NodeCapability)
Q_DECLARE_OPERATORS_FOR_FLAGS(NodeCapabilities)

struct KoColorConversionSystem::Node : boost::equality_comparable<KoColorConversionSystem::Node>
{

    Node()
        : isInitialized(false)
        , referenceDepth(0)
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
            {
                const bool isGray = modelId == GrayColorModelID.id() ||
                        modelId == GrayAColorModelID.id();

                const bool isAlpha = modelId == AlphaColorModelID.id();

                if (_colorSpaceFactory->isHdr()) {
                    m_capabilities |= HasHdr;
                }

                if (!isGray && !isAlpha) {
                    m_capabilities |= HasColor;
                }

                if (!isAlpha && modelId != GrayColorModelID.id()) {
                    m_capabilities |= HasAlpha;
                }
            }

            colorSpaceFactory = _colorSpaceFactory;
            referenceDepth = _colorSpaceFactory->referenceDepth();
            crossingCost = _colorSpaceFactory->crossingCost();
        }
    }

    void init(const KoColorSpaceEngine* _engine) {
        Q_ASSERT(!isInitialized);
        isEngine = true;
        isInitialized = true;
        engine = _engine;
        m_capabilities = HasAlpha | HasColor | HasHdr;
    }

    QString id() const {
        return modelId + " " + depthId + " " + profileName;
    }

    NodeKey key() const;

    friend bool operator==(const Node &lhs, const Node &rhs) {
        return lhs.modelId == rhs.modelId &&
            lhs.depthId == rhs.depthId &&
            lhs.profileName == rhs.profileName;
    }

    NodeCapabilities capabilities() const {
        return m_capabilities;
    }

    QString modelId;
    QString depthId;
    QString profileName;
    bool isInitialized;
    int referenceDepth;
    QList<Vertex*> outputVertexes;
    int crossingCost;
    const KoColorSpaceFactory* colorSpaceFactory;
    bool isEngine;
    const KoColorSpaceEngine* engine;
    NodeCapabilities m_capabilities = None;
};

Q_DECLARE_TYPEINFO(KoColorConversionSystem::Node, Q_MOVABLE_TYPE);

QDebug operator<<(QDebug dbg, const KoColorConversionSystem::Node &node)
{
    dbg.nospace() << "Node(" << node.modelId << ", " << node.depthId << ", " << node.profileName << ")";
    return dbg.space();
}

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
    }

    void setFactoryFromDst(KoColorConversionTransformationFactory* factory) {
        factoryFromDst = factory;
    }

    KoColorConversionTransformationFactory* factory() {
        if (factoryFromSrc) return factoryFromSrc;
        return factoryFromDst;
    }

    Node* srcNode;
    Node* dstNode;

private:

    KoColorConversionTransformationFactory* factoryFromSrc; // Factory provided by the destination node
    KoColorConversionTransformationFactory* factoryFromDst; // Factory provided by the destination node

};

struct KoColorConversionSystem::NodeKey
        : public boost::equality_comparable<NodeKey>
{

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

QDebug operator<<(QDebug dbg, const KoColorConversionSystem::NodeKey &key)
{
    dbg.nospace() << "NodeKey(" << key.modelId << ", " << key.depthId << ", " << key.profileName << ")";
    return dbg.space();
}


inline KoColorConversionSystem::NodeKey KoColorConversionSystem::Node::key() const
{
    return NodeKey(modelId, depthId, profileName);
}

struct KoColorConversionSystem::Path {

    Path()
        : referenceDepth(0)
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
            commonNodeCapabilities = v->srcNode->capabilities();
        }

        commonNodeCapabilities &= v->dstNode->capabilities();

        vertexes.append(v);

        referenceDepth = qMin(referenceDepth, v->dstNode->referenceDepth);
        cost += v->dstNode->crossingCost;
    }

    NodeCapabilities unsupportedCapabilities() const {
        NodeCapabilities minimalCaps =
            !vertexes.isEmpty() ?
            vertexes.first()->srcNode->capabilities() &
                vertexes.last()->dstNode->capabilities() :
            None;

        return (minimalCaps & commonNodeCapabilities) ^ minimalCaps;
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
    int referenceDepth;
    bool isGood;
    int cost;
    NodeCapabilities commonNodeCapabilities = None;

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

struct PathQualityChecker {

    PathQualityChecker(int _referenceDepth)
        : referenceDepth(_referenceDepth)
    {}

    /// @return true if the path maximize all the criteria (except length)
    inline bool isGoodPath(const KoColorConversionSystem::Path & path) const {
        return path.unsupportedCapabilities() == None &&
            path.referenceDepth >= referenceDepth;
    }

    /**
     * Compare two paths.
     */
    inline bool lessWorseThan(const KoColorConversionSystem::Path &path1, const KoColorConversionSystem::Path &path2) const {
        // There is no point in comparing two paths which doesn't start from the same node or doesn't end at the same node

        NodeCapabilities unsupported1 = path1.unsupportedCapabilities();
        NodeCapabilities unsupported2 = path2.unsupportedCapabilities();

        if (!unsupported1.testFlag(HasHdr) && unsupported2.testFlag(HasHdr)) {
            return true;
        }

        if (!unsupported1.testFlag(HasColor) && unsupported2.testFlag(HasColor)) {
            return true;
        }

        if (!unsupported1.testFlag(HasAlpha) && unsupported2.testFlag(HasAlpha)) {
            return true;
        }

        if (path1.referenceDepth == path2.referenceDepth) {
            return path1.cost < path2.cost; // if they have the same cost, well anyway you have to choose one, and there is no point in keeping one and not the other
        }
        return path1.referenceDepth > path2.referenceDepth;
    }
    int referenceDepth;
};

#undef CHECK_ONE_AND_NOT_THE_OTHER

#endif
