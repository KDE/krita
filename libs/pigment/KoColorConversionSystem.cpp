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

#include "KoColorConversionSystem.h"
#include "KoColorConversionSystem_p.h"

#include <QHash>
#include <QString>

#include "KoColorConversionAlphaTransformation.h"
#include "KoColorConversionTransformation.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoCopyColorConversionTransformation.h"
#include "KoMultipleColorConversionTransformation.h"


KoColorConversionSystem::KoColorConversionSystem(RegistryInterface *registryInterface)
    : d(new Private(registryInterface))
{
}

KoColorConversionSystem::~KoColorConversionSystem()
{
    qDeleteAll(d->graph);
    qDeleteAll(d->vertexes);
    delete d;
}

void KoColorConversionSystem::connectToEngine(Node* _node, Node* _engine)
{
    Vertex* v1 = createVertex(_node, _engine);
    Vertex* v2 = createVertex(_engine, _node);
    v1->conserveColorInformation = !_node->isGray;
    v2->conserveColorInformation = !_node->isGray;
    v1->conserveDynamicRange = _engine->isHdr;
    v2->conserveDynamicRange = _engine->isHdr;
}

KoColorConversionSystem::Node* KoColorConversionSystem::insertEngine(const KoColorSpaceEngine* engine)
{
    NodeKey key(engine->id(), engine->id(), engine->id());
    Node* n = new Node;
    n->modelId = engine->id();
    n->depthId = engine->id();
    n->profileName = engine->id();
    n->referenceDepth = 64; // engine don't have reference depth,
    d->graph.insert(key, n);
    n->init(engine);
    return n;
}


void KoColorConversionSystem::insertColorSpace(const KoColorSpaceFactory* csf)
{
    dbgPigment << "Inserting color space " << csf->name() << " (" << csf->id() << ") Model: " << csf->colorModelId() << " Depth: " << csf->colorDepthId() << " into the CCS";
    const QList<const KoColorProfile*> profiles = d->registryInterface->profilesFor(csf);
    QString modelId = csf->colorModelId().id();
    QString depthId = csf->colorDepthId().id();
    if (profiles.isEmpty()) { // There is no profile for this CS, create a node without profile name if the color engine isn't icc-based
        if (csf->colorSpaceEngine() != "icc") {
            Node* n = nodeFor(modelId, depthId, "default");
            n->init(csf);
        }
        else {
            dbgPigment << "Cannot add node for " << csf->name() << ", since there are no profiles available";
        }
    } else {
        // Initialise the nodes
        Q_FOREACH (const KoColorProfile* profile, profiles) {
            Node* n = nodeFor(modelId, depthId, profile->name());
            n->init(csf);
            if (!csf->colorSpaceEngine().isEmpty()) {
                KoColorSpaceEngine* engine = KoColorSpaceEngineRegistry::instance()->get(csf->colorSpaceEngine());
                Q_ASSERT(engine);
                NodeKey engineKey(engine->id(), engine->id(), engine->id());
                Node* engineNode = 0;
                QHash<NodeKey, Node*>::ConstIterator it = d->graph.constFind(engineKey);
                if (it != d->graph.constEnd()) {
                    engineNode = it.value();
                } else {
                    engineNode = insertEngine(engine);
                }

                if (engine->supportsColorSpace(modelId, depthId, profile)) {
                    connectToEngine(n, engineNode);
                }
            }
        }
    }
    // Construct a link for "custom" transformation
    const QList<KoColorConversionTransformationFactory*> cctfs = csf->colorConversionLinks();
    Q_FOREACH (KoColorConversionTransformationFactory* cctf, cctfs) {
        Node* srcNode = nodeFor(cctf->srcColorModelId(), cctf->srcColorDepthId(), cctf->srcProfile());
        Q_ASSERT(srcNode);
        Node* dstNode = nodeFor(cctf->dstColorModelId(), cctf->dstColorDepthId(), cctf->dstProfile());
        Q_ASSERT(dstNode);
        // Check if the two nodes are already connected
        Vertex* v = vertexBetween(srcNode, dstNode);
        // If the vertex doesn't already exist, then create it
        if (!v) {
            v = createVertex(srcNode, dstNode);
        }
        Q_ASSERT(v); // we should have one now
        if (dstNode->modelId == modelId && dstNode->depthId == depthId) {
            v->setFactoryFromDst(cctf);
        }
        if (srcNode->modelId == modelId && srcNode->depthId == depthId) {
            v->setFactoryFromSrc(cctf);
        }
    }
}

void KoColorConversionSystem::insertColorProfile(const KoColorProfile* _profile)
{
    dbgPigmentCCS << _profile->name();
    const QList< const KoColorSpaceFactory* >& factories = d->registryInterface->colorSpacesFor(_profile);
    Q_FOREACH (const KoColorSpaceFactory* factory, factories) {
        QString modelId = factory->colorModelId().id();
        QString depthId = factory->colorDepthId().id();
        Node* n = nodeFor(modelId, depthId, _profile->name());
        n->init(factory);
        if (!factory->colorSpaceEngine().isEmpty()) {
            KoColorSpaceEngine* engine = KoColorSpaceEngineRegistry::instance()->get(factory->colorSpaceEngine());
            Q_ASSERT(engine);
            Node* engineNode = d->graph[ NodeKey(engine->id(), engine->id(), engine->id())];
            Q_ASSERT(engineNode);

            if (engine->supportsColorSpace(modelId, depthId, _profile)) {
                connectToEngine(n, engineNode);
            }
        }
        const QList<KoColorConversionTransformationFactory*> cctfs = factory->colorConversionLinks();
        Q_FOREACH (KoColorConversionTransformationFactory* cctf, cctfs) {
            Node* srcNode = nodeFor(cctf->srcColorModelId(), cctf->srcColorDepthId(), cctf->srcProfile());
            Q_ASSERT(srcNode);
            Node* dstNode = nodeFor(cctf->dstColorModelId(), cctf->dstColorDepthId(), cctf->dstProfile());
            Q_ASSERT(dstNode);
            if (srcNode == n || dstNode == n) {
                // Check if the two nodes are already connected
                Vertex* v = vertexBetween(srcNode, dstNode);
                // If the vertex doesn't already exist, then create it
                if (!v) {
                    v = createVertex(srcNode, dstNode);
                }
                Q_ASSERT(v); // we should have one now
                if (dstNode->modelId == modelId && dstNode->depthId == depthId) {
                    v->setFactoryFromDst(cctf);
                }
                if (srcNode->modelId == modelId && srcNode->depthId == depthId) {
                    v->setFactoryFromSrc(cctf);
                }
            }
        }
    }
}

const KoColorSpace* KoColorConversionSystem::defaultColorSpaceForNode(const Node* node) const
{
    return d->registryInterface->colorSpace(node->modelId, node->depthId, node->profileName);
}

KoColorConversionSystem::Node* KoColorConversionSystem::createNode(const QString& _modelId, const QString& _depthId, const QString& _profileName)
{
    Node* n = new Node;
    n->modelId = _modelId;
    n->depthId = _depthId;
    n->profileName = _profileName;
    d->graph.insert(NodeKey(_modelId, _depthId, _profileName), n);
    return n;
}

const KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const KoColorSpace* _colorSpace) const
{
    const KoColorProfile* profile = _colorSpace->profile();
    return nodeFor(_colorSpace->colorModelId().id(), _colorSpace->colorDepthId().id(),
                   profile ? profile->name() : "default");
}

const KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const QString& _colorModelId, const QString& _colorDepthId, const QString& _profileName) const
{
    dbgPigmentCCS << "Look for node: " << _colorModelId << " " << _colorDepthId << " " << _profileName;
    return nodeFor(NodeKey(_colorModelId, _colorDepthId, _profileName));
}

const KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const NodeKey& key) const
{
    dbgPigmentCCS << "Look for node: " << key.modelId << " " << key.depthId << " " << key.profileName << " " << d->graph.value(key);
    return d->graph.value(key);
}

KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const QString& _colorModelId, const QString& _colorDepthId, const QString& _profileName)
{
    return nodeFor(NodeKey(_colorModelId, _colorDepthId, _profileName));
}

KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const KoColorConversionSystem::NodeKey& key)
{
    QHash<NodeKey, Node*>::ConstIterator it = d->graph.constFind(key);
    if (it != d->graph.constEnd()) {
        return it.value();
    } else {
        return createNode(key.modelId, key.depthId, key.profileName);
    }
}

QList<KoColorConversionSystem::Node*> KoColorConversionSystem::nodesFor(const QString& _modelId, const QString& _depthId)
{
    QList<Node*> nodes;
    Q_FOREACH (Node* node, d->graph) {
        if (node->modelId == _modelId &&  node->depthId == _depthId) {
            nodes << node;
        }
    }
    return nodes;
}

KoColorConversionTransformation* KoColorConversionSystem::createColorConverter(const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    if (*srcColorSpace == *dstColorSpace) {
        return new KoCopyColorConversionTransformation(srcColorSpace);
    }
    Q_ASSERT(srcColorSpace);
    Q_ASSERT(dstColorSpace);
    dbgPigmentCCS << srcColorSpace->id() << (srcColorSpace->profile() ? srcColorSpace->profile()->name() : "default");
    dbgPigmentCCS << dstColorSpace->id() << (dstColorSpace->profile() ? dstColorSpace->profile()->name() : "default");
    Path path = findBestPath(
                nodeFor(srcColorSpace),
                nodeFor(dstColorSpace));
    Q_ASSERT(path.length() > 0);
    KoColorConversionTransformation* transfo = createTransformationFromPath(path, srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    Q_ASSERT(*transfo->srcColorSpace() == *srcColorSpace);
    Q_ASSERT(*transfo->dstColorSpace() == *dstColorSpace);
    Q_ASSERT(transfo);
    return transfo;
}

void KoColorConversionSystem::createColorConverters(const KoColorSpace* colorSpace, const QList< QPair<KoID, KoID> >& possibilities, KoColorConversionTransformation*& fromCS, KoColorConversionTransformation*& toCS) const
{
    // TODO This function currently only select the best conversion only based on the transformation
    // from colorSpace to one of the color spaces in the list, but not the other way around
    // it might be worth to look also the return path.
    const Node* csNode = nodeFor(colorSpace);
    PathQualityChecker pQC(csNode->referenceDepth, !csNode->isHdr, !csNode->isGray);
    // Look for a color conversion
    Path bestPath;
    typedef QPair<KoID, KoID> KoID2KoID;
    Q_FOREACH (const KoID2KoID & possibility, possibilities) {
        const KoColorSpaceFactory* csf = d->registryInterface->colorSpaceFactory(possibility.first.id(), possibility.second.id());
        if (csf) {
            Path path = findBestPath(csNode, nodeFor(csf->colorModelId().id(), csf->colorDepthId().id(), csf->defaultProfile()));
            Q_ASSERT(path.length() > 0);
            path.isGood = pQC.isGoodPath(path);

            if (bestPath.isEmpty()) {
                bestPath = path;
            } else if ((!bestPath.isGood &&  path.isGood) || pQC.lessWorseThan(path, bestPath)) {
                bestPath = path;
            }
        }
    }
    Q_ASSERT(!bestPath.isEmpty());
    const KoColorSpace* endColorSpace = defaultColorSpaceForNode(bestPath.endNode());
    fromCS = createTransformationFromPath(bestPath, colorSpace, endColorSpace, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    Path returnPath = findBestPath(bestPath.endNode(), csNode);
    Q_ASSERT(!returnPath.isEmpty());
    toCS = createTransformationFromPath(returnPath, endColorSpace, colorSpace, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    Q_ASSERT(*toCS->dstColorSpace() == *fromCS->srcColorSpace());
    Q_ASSERT(*fromCS->dstColorSpace() == *toCS->srcColorSpace());
}

KoColorConversionTransformation* KoColorConversionSystem::createTransformationFromPath(const Path &path, const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_ASSERT(srcColorSpace->colorModelId().id() == path.startNode()->modelId);
    Q_ASSERT(srcColorSpace->colorDepthId().id() == path.startNode()->depthId);
    Q_ASSERT(dstColorSpace->colorModelId().id() == path.endNode()->modelId);
    Q_ASSERT(dstColorSpace->colorDepthId().id() == path.endNode()->depthId);

    KoColorConversionTransformation* transfo;

    const QList< Path::node2factory > pathOfNode = path.compressedPath();

    if (pathOfNode.size() == 2) { // Direct connection
        transfo = pathOfNode[1].second->createColorTransformation(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    }
    else {

        KoMultipleColorConversionTransformation* mccTransfo = new KoMultipleColorConversionTransformation(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

        transfo = mccTransfo;

        // Get the first intermediary color space
        dbgPigmentCCS << pathOfNode[ 0 ].first->id() << " to " << pathOfNode[ 1 ].first->id();

        const KoColorSpace* intermCS =
                defaultColorSpaceForNode(pathOfNode[1].first);

        mccTransfo->appendTransfo(pathOfNode[1].second->createColorTransformation(srcColorSpace, intermCS, renderingIntent, conversionFlags));

        for (int i = 2; i < pathOfNode.size() - 1; i++) {
            dbgPigmentCCS << pathOfNode[ i - 1 ].first->id() << " to " << pathOfNode[ i ].first->id();
            const KoColorSpace* intermCS2 = defaultColorSpaceForNode(pathOfNode[i].first);
            Q_ASSERT(intermCS2);
            mccTransfo->appendTransfo(pathOfNode[i].second->createColorTransformation(intermCS, intermCS2, renderingIntent, conversionFlags));
            intermCS = intermCS2;
        }

        dbgPigmentCCS << pathOfNode[ pathOfNode.size() - 2 ].first->id() << " to " << pathOfNode[ pathOfNode.size() - 1 ].first->id();
        mccTransfo->appendTransfo(pathOfNode.last().second->createColorTransformation(intermCS, dstColorSpace, renderingIntent, conversionFlags));
    }
    return transfo;
}


KoColorConversionSystem::Vertex* KoColorConversionSystem::vertexBetween(KoColorConversionSystem::Node* srcNode, KoColorConversionSystem::Node* dstNode)
{
    Q_FOREACH (Vertex* oV, srcNode->outputVertexes) {
        if (oV->dstNode == dstNode) {
            return oV;
        }
    }
    return 0;
}

KoColorConversionSystem::Vertex* KoColorConversionSystem::createVertex(Node* srcNode, Node* dstNode)
{
    Vertex* v = new Vertex(srcNode, dstNode);
    srcNode->outputVertexes.append(v);
    d->vertexes.append(v);
    return v;
}

// -- Graph visualization functions --

QString KoColorConversionSystem::vertexToDot(KoColorConversionSystem::Vertex* v, const QString &options) const
{
    return QString("  \"%1\" -> \"%2\" %3\n").arg(v->srcNode->id()).arg(v->dstNode->id()).arg(options);
}

QString KoColorConversionSystem::toDot() const
{
    QString dot = "digraph CCS {\n";
    Q_FOREACH (Vertex* oV, d->vertexes) {
        dot += vertexToDot(oV, "default") ;
    }
    dot += "}\n";
    return dot;
}

bool KoColorConversionSystem::existsPath(const QString& srcModelId, const QString& srcDepthId, const QString& srcProfileName, const QString& dstModelId, const QString& dstDepthId, const QString& dstProfileName) const
{
    dbgPigmentCCS << "srcModelId = " << srcModelId << " srcDepthId = " << srcDepthId << " srcProfileName = " << srcProfileName << " dstModelId = " << dstModelId << " dstDepthId = " << dstDepthId << " dstProfileName = " << dstProfileName;
    const Node* srcNode = nodeFor(srcModelId, srcDepthId, srcProfileName);
    const Node* dstNode = nodeFor(dstModelId, dstDepthId, dstProfileName);
    if (srcNode == dstNode) return true;
    if (!srcNode) return false;
    if (!dstNode) return false;
    Path path = findBestPath(srcNode, dstNode);
    bool exist = !path.isEmpty();
    return exist;
}

bool KoColorConversionSystem::existsGoodPath(const QString& srcModelId, const QString& srcDepthId, const QString& srcProfileName, const QString& dstModelId, const QString& dstDepthId, const QString& dstProfileName) const
{
    const Node* srcNode = nodeFor(srcModelId, srcDepthId, srcProfileName);
    const Node* dstNode = nodeFor(dstModelId, dstDepthId, dstProfileName);
    if (srcNode == dstNode) return true;
    if (!srcNode) return false;
    if (!dstNode) return false;
    Path path = findBestPath(srcNode, dstNode);
    bool existAndGood = path.isGood;
    return existAndGood;
}


QString KoColorConversionSystem::bestPathToDot(const QString& srcKey, const QString& dstKey) const
{
    const Node* srcNode = 0;
    const Node* dstNode = 0;
    Q_FOREACH (Node* node, d->graph) {
        if (node->id() == srcKey) {
            srcNode = node;
        }
        if (node->id() == dstKey) {
            dstNode = node;
        }
    }
    Path p = findBestPath(srcNode, dstNode);
    Q_ASSERT(!p.isEmpty());
    QString dot = "digraph CCS {\n" +
            QString("  \"%1\" [color=red]\n").arg(srcNode->id()) +
            QString("  \"%1\" [color=red]\n").arg(dstNode->id());
    Q_FOREACH (Vertex* oV, d->vertexes) {
        QString options;
        if (p.vertexes.contains(oV)) {
            options = "[color=red]";
        }
        dot += vertexToDot(oV, options) ;
    }
    dot += "}\n";
    return dot;
}

inline KoColorConversionSystem::Path KoColorConversionSystem::findBestPathImpl2(const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode, bool ignoreHdr, bool ignoreColorCorrectness) const
{
    PathQualityChecker pQC(qMin(srcNode->referenceDepth, dstNode->referenceDepth), ignoreHdr, ignoreColorCorrectness);
    Node2PathHash node2path; // current best path to reach a given node
    QList<Path> possiblePaths; // list of all paths
    // Generate the initial list of paths
    Q_FOREACH (Vertex* v, srcNode->outputVertexes) {
        if (v->dstNode->isInitialized) {
            Path p;
            p.appendVertex(v);
            Node* endNode = v->dstNode;
            if (endNode == dstNode) {
                Q_ASSERT(pQC.isGoodPath(p));  // <- it's a direct link, it has to be a good path
                p.isGood = true;
                return p;
            } else {
                //Q_ASSERT(!node2path.contains(endNode));   // That would be a total fuck up if there are two vertices between two nodes
                node2path.insert(endNode, p);
                possiblePaths.append(p);
            }
        }
    }

    Path currentBestPath;
    // Continue while there are any possibilities remaining
    while (possiblePaths.size() > 0) {

        // Loop through all paths and explore one step further
        const QList<Path> currentPaths = possiblePaths;
        for (const Path &p : currentPaths) {
            const Node* endNode = p.endNode();
            for (Vertex* v : endNode->outputVertexes) {
                if (v->dstNode->isInitialized && !p.contains(v->dstNode)) {
                    Path newP = p;  // Candidate
                    newP.appendVertex(v);
                    Node* newEndNode = v->dstNode;
                    if (newEndNode == dstNode) {
                        if (pQC.isGoodPath(newP)) { // Victory
                            newP.isGood = true;
                            return newP;
                        } else if (pQC.lessWorseThan(newP, currentBestPath)) {
                            if (newP.startNode() && newP.endNode() && currentBestPath.startNode() && currentBestPath.endNode()) {
                                Q_ASSERT(newP.startNode()->id() == currentBestPath.startNode()->id());
                                Q_ASSERT(newP.endNode()->id() == currentBestPath.endNode()->id());
                                // Can we do better than dumping memory values???
                                // warnPigment << pQC.lessWorseThan(newP, currentBestPath) << " " << newP << "  " << currentBestPath;
                                currentBestPath = newP;
                            }
                        }
                    } else {
                        // This is an incomplete path. Check if there's a better way to get to its endpoint.
                        Node2PathHash::Iterator it = node2path.find(newEndNode);
                        if (it != node2path.end()) {
                            Path &p2 = it.value();
                            if (pQC.lessWorseThan(newP, p2)) {
                                p2 = newP;
                                possiblePaths.append(newP);
                            }
                        } else {
                            node2path.insert(newEndNode, newP);
                            possiblePaths.append(newP);
                        }
                    }
                }
            }
            possiblePaths.removeAll(p); // Remove from list of remaining paths
        }
    }
    if (!currentBestPath.isEmpty()) {
        warnPigment << "No good path from " << srcNode->id() << " to " << dstNode->id() << " found : length = " << currentBestPath.length() << " cost = " << currentBestPath.cost << " referenceDepth = " << currentBestPath.referenceDepth << " respectColorCorrectness = " << currentBestPath.respectColorCorrectness << " isGood = " << currentBestPath.isGood ;
        return currentBestPath;
    }
    errorPigment << "No path from " << srcNode->id() << " to " << dstNode->id() << " found not ";
    return currentBestPath;
}

inline KoColorConversionSystem::Path KoColorConversionSystem::findBestPathImpl(const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode, bool ignoreHdr) const
{
    Q_ASSERT(srcNode);
    Q_ASSERT(dstNode);
    return findBestPathImpl2(srcNode, dstNode, ignoreHdr, (srcNode->isGray || dstNode->isGray));
}

KoColorConversionSystem::Path KoColorConversionSystem::findBestPath(const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
{
    Q_ASSERT(srcNode);
    Q_ASSERT(dstNode);
    dbgPigmentCCS << "Find best path between " << srcNode->id() << " and  " << dstNode->id();
    if (srcNode->isHdr &&  dstNode->isHdr) {
        return findBestPathImpl(srcNode, dstNode, false);
    } else {
        return findBestPathImpl(srcNode, dstNode, true);
    }
}
