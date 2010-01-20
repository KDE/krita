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

#include "KoColorConversionSystem.h"

#include <QHash>
#include <QString>

#include "DebugPigment.h"

#include "KoColorConversionAlphaTransformation.h"
#include "KoColorConversionTransformation.h"
#include "KoColorConversionTransformationFactory.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoColorSpaceEngine.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorModelStandardIds.h"
#include "KoCopyColorConversionTransformation.h"
#include "KoMultipleColorConversionTransformation.h"

#include "KoColorConversionSystem_p.h"

KoColorConversionSystem::KoColorConversionSystem() : d(new Private)
{
    // Create the Alpha 8bit
    d->alphaNode = new Node;
    d->alphaNode->modelId = AlphaColorModelID.id();
    d->alphaNode->depthId = Integer8BitsColorDepthID.id();
    d->alphaNode->crossingCost = 1000000;
    d->alphaNode->isInitialized = true;
    d->alphaNode->isGray = true; // <- FIXME: it's a little bit hacky as alpha doesn't really have color information
    d->graph[ NodeKey(d->alphaNode->modelId, d->alphaNode->depthId, "Dummy profile")] = d->alphaNode;

    Vertex* v = createVertex(d->alphaNode, d->alphaNode);
    v->setFactoryFromSrc(new KoCopyColorConversionTransformationFactory(AlphaColorModelID.id(), Integer8BitsColorDepthID.id(), ""));
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
    v1->conserveDynamicRange = !_node->isHdr;
    v2->conserveDynamicRange = !_node->isHdr;
}

KoColorConversionSystem::Node* KoColorConversionSystem::insertEngine(const KoColorSpaceEngine* engine)
{
    NodeKey key(engine->id(), engine->id(), engine->id());
    Node* n = new Node;
    n->modelId = engine->id();
    n->depthId = engine->id();
    n->profileName = engine->id();
    n->referenceDepth = 64; // engine don't have reference depth,
    d->graph[ key ] = n;
    n->init(engine);
    return n;
}


void KoColorConversionSystem::insertColorSpace(const KoColorSpaceFactory* csf)
{
    dbgPigmentCCS << "Inserting color space " << csf->name() << " (" << csf->id() << ") Model: " << csf->colorModelId() << " Depth: " << csf->colorDepthId() << " into the CCS";
    QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(csf);
    QString modelId = csf->colorModelId().id();
    QString depthId = csf->colorDepthId().id();
    if (profiles.isEmpty()) { // There is no profile for this CS, create a node without profile name
        Q_ASSERT(csf->colorSpaceEngine() != "icc");
        Node* n = nodeFor(modelId, depthId, "");
        n->init(csf);
    } else {
        // Initialise the nodes
        foreach(const KoColorProfile* profile, profiles) {
            Node* n = nodeFor(modelId, depthId, profile->name());
            n->init(csf);
            if (!csf->colorSpaceEngine().isEmpty()) {
                KoColorSpaceEngine* engine = KoColorSpaceEngineRegistry::instance()->get(csf->colorSpaceEngine());
                Q_ASSERT(engine);
                NodeKey engineKey(engine->id(), engine->id(), engine->id());
                Node* engineNode = 0;
                if (d->graph.contains(engineKey)) {
                    engineNode = d->graph[engineKey];
                } else {
                    engineNode = insertEngine(engine);
                }
                connectToEngine(n, engineNode);
            }
        }
    }
    // Construct a link for "custom" transformation
    QList<KoColorConversionTransformationFactory*> cctfs = csf->colorConversionLinks();
    foreach(KoColorConversionTransformationFactory* cctf, cctfs) {
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
    const QList< const KoColorSpaceFactory* >& factories = KoColorSpaceRegistry::instance()->colorSpacesFor(_profile);
    foreach(const KoColorSpaceFactory* factory, factories) {
        QString modelId = factory->colorModelId().id();
        QString depthId = factory->colorDepthId().id();
        Node* n = nodeFor(modelId, depthId, _profile->name());
        n->init(factory);
        if (!factory->colorSpaceEngine().isEmpty()) {
            KoColorSpaceEngine* engine = KoColorSpaceEngineRegistry::instance()->get(factory->colorSpaceEngine());
            Q_ASSERT(engine);
            Node* engineNode = d->graph[ NodeKey(engine->id(), engine->id(), engine->id())];
            Q_ASSERT(engineNode);
            connectToEngine(n, engineNode);
        }
        QList<KoColorConversionTransformationFactory*> cctfs = factory->colorConversionLinks();
        foreach(KoColorConversionTransformationFactory* cctf, cctfs) {
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
    return KoColorSpaceRegistry::instance()->colorSpace(node->modelId, node->depthId, node->profileName);
}

KoColorConversionSystem::Node* KoColorConversionSystem::createNode(const QString& _modelId, const QString& _depthId, const QString& _profileName)
{
    Node* n = new Node;
    n->modelId = _modelId;
    n->depthId = _depthId;
    n->profileName = _profileName;
    d->graph[ NodeKey(_modelId, _depthId, _profileName)] = n;
    Q_ASSERT(vertexBetween(d->alphaNode, n) == 0); // The two color spaces should not be connected yet
    Vertex* vFromAlpha = createVertex(d->alphaNode, n);
    vFromAlpha->setFactoryFromSrc(new KoColorConversionFromAlphaTransformationFactory(_modelId, _depthId, _profileName));
    Q_ASSERT(vertexBetween(n, d->alphaNode) == 0); // The two color spaces should not be connected yet
    Vertex* vToAlpha = createVertex(n, d->alphaNode);
    vToAlpha->setFactoryFromDst(new KoColorConversionToAlphaTransformationFactory(_modelId, _depthId, _profileName));
    return n;
}

const KoColorConversionSystem::Node* KoColorConversionSystem::nodeFor(const KoColorSpace* _colorSpace) const
{
    const KoColorProfile* profile = _colorSpace->profile();
    return nodeFor(_colorSpace->colorModelId().id(), _colorSpace->colorDepthId().id(),
                   profile ? profile->name() : "");
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
    if (d->graph.contains(key)) {
        return d->graph.value(key);
    } else {
        return createNode(key.modelId, key.depthId, key.profileName);
    }
}

QList<KoColorConversionSystem::Node*> KoColorConversionSystem::nodesFor(const QString& _modelId, const QString& _depthId)
{
    QList<Node*> nodes;
    foreach(Node* node, d->graph) {
        if (node->modelId == _modelId &&  node->depthId == _depthId) {
            nodes << node;
        }
    }
    return nodes;
}

KoColorConversionTransformation* KoColorConversionSystem::createColorConverter(const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    if (*srcColorSpace == *dstColorSpace) {
        return new KoCopyColorConversionTransformation(srcColorSpace);
    }
    Q_ASSERT(srcColorSpace);
    Q_ASSERT(dstColorSpace);
    dbgPigmentCCS << srcColorSpace->id() << (srcColorSpace->profile() ? srcColorSpace->profile()->name() : "");
    dbgPigmentCCS << dstColorSpace->id() << (dstColorSpace->profile() ? dstColorSpace->profile()->name() : "");
    Path* path = findBestPath(
                     nodeFor(srcColorSpace),
                     nodeFor(dstColorSpace));
    Q_ASSERT(path);
    KoColorConversionTransformation* transfo = createTransformationFromPath(path, srcColorSpace, dstColorSpace, renderingIntent);
    delete path;
    Q_ASSERT(*transfo->srcColorSpace() == *srcColorSpace);
    Q_ASSERT(*transfo->dstColorSpace() == *dstColorSpace);
    Q_ASSERT(transfo);
    return transfo;
}

void KoColorConversionSystem::createColorConverters(const KoColorSpace* colorSpace, QList< QPair<KoID, KoID> >& possibilities, KoColorConversionTransformation*& fromCS, KoColorConversionTransformation*& toCS) const
{
    // TODO This function currently only select the best conversion only based on the transformation
    // from colorSpace to one of the color spaces in the list, but not the other way around
    // it might be worth to look also the return path.
    const Node* csNode = nodeFor(colorSpace);
    PathQualityChecker pQC(csNode->referenceDepth, !csNode->isHdr, !csNode->isGray);
    // Look for a color conversion
    Path* bestPath = 0;
    typedef QPair<KoID, KoID> KoID2KoID;
    foreach(const KoID2KoID & possibility, possibilities) {
        const KoColorSpaceFactory* csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(KoColorSpaceRegistry::instance()->colorSpaceId(possibility.first.id(), possibility.second.id()));
        if (csf) {
            Path* path = findBestPath(csNode, nodeFor(csf->colorModelId().id(), csf->colorDepthId().id(), csf->defaultProfile()));
            Q_ASSERT(path);
            path->isGood = pQC.isGoodPath(path);

            if (!bestPath) {
                bestPath = path;
            } else if ((!bestPath->isGood &&  path->isGood) || pQC.lessWorseThan(path, bestPath)) {
                delete bestPath;
                bestPath = path;
            } else {
                delete path;
            }
        }
    }
    Q_ASSERT(bestPath);
    const KoColorSpace* endColorSpace = defaultColorSpaceForNode(bestPath->endNode());
    fromCS = createTransformationFromPath(bestPath, colorSpace, endColorSpace);
    Path* returnPath = findBestPath(bestPath->endNode(), csNode);
    Q_ASSERT(returnPath);
    toCS = createTransformationFromPath(returnPath, endColorSpace, colorSpace);
    Q_ASSERT(*toCS->dstColorSpace() == *fromCS->srcColorSpace());
    Q_ASSERT(*fromCS->dstColorSpace() == *toCS->srcColorSpace());
}

KoColorConversionTransformation* KoColorConversionSystem::createTransformationFromPath(const Path* path, const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    Q_ASSERT(srcColorSpace->colorModelId().id() == path->startNode()->modelId);
    Q_ASSERT(srcColorSpace->colorDepthId().id() == path->startNode()->depthId);
    Q_ASSERT(dstColorSpace->colorModelId().id() == path->endNode()->modelId);
    Q_ASSERT(dstColorSpace->colorDepthId().id() == path->endNode()->depthId);
    KoColorConversionTransformation* transfo;
    QList< Path::node2factory > pathOfNode = path->compressedPath();
    if (pathOfNode.size() == 2) { // Direct connection
        transfo = pathOfNode[1].second->createColorTransformation(srcColorSpace, dstColorSpace, renderingIntent);
    } else {
        KoMultipleColorConversionTransformation* mccTransfo = new KoMultipleColorConversionTransformation(srcColorSpace, dstColorSpace, renderingIntent);
        transfo = mccTransfo;
        // Get the first intermediary color space
        const KoColorSpace* intermCS =
            defaultColorSpaceForNode(pathOfNode[1].first);
        mccTransfo->appendTransfo(pathOfNode[1].second->createColorTransformation(srcColorSpace, intermCS, renderingIntent));
        dbgPigmentCCS << pathOfNode[ 0 ].first->id() << " to " << pathOfNode[ 1 ].first->id();
        for (int i = 2; i < pathOfNode.size() - 1; i++) {
            dbgPigmentCCS << pathOfNode[ i - 1 ].first->id() << " to " << pathOfNode[ i ].first->id();
            const KoColorSpace* intermCS2 = defaultColorSpaceForNode(pathOfNode[i].first);
            Q_ASSERT(intermCS2);
            mccTransfo->appendTransfo(pathOfNode[i].second->createColorTransformation(intermCS, intermCS2, renderingIntent));
            intermCS = intermCS2;
        }
        dbgPigmentCCS << pathOfNode[ pathOfNode.size() - 2 ].first->id() << " to " << pathOfNode[ pathOfNode.size() - 1 ].first->id();
        mccTransfo->appendTransfo(pathOfNode.last().second->createColorTransformation(intermCS, dstColorSpace, renderingIntent));
    }
    return transfo;
}


KoColorConversionSystem::Vertex* KoColorConversionSystem::vertexBetween(KoColorConversionSystem::Node* srcNode, KoColorConversionSystem::Node* dstNode)
{
    foreach(Vertex* oV, srcNode->outputVertexes) {
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

QString KoColorConversionSystem::vertexToDot(KoColorConversionSystem::Vertex* v, QString options) const
{
    return QString("  \"%1\" -> \"%2\" %3\n").arg(v->srcNode->id()).arg(v->dstNode->id()).arg(options);
}

QString KoColorConversionSystem::toDot() const
{
    QString dot = "digraph CCS {\n";
    foreach(Vertex* oV, d->vertexes) {
        dot += vertexToDot(oV, "") ;
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
    Path* path = findBestPath(srcNode, dstNode);
    bool exist = path;
    delete path;
    return exist;
}

bool KoColorConversionSystem::existsGoodPath(const QString& srcModelId, const QString& srcDepthId, const QString& srcProfileName, const QString& dstModelId, const QString& dstDepthId, const QString& dstProfileName) const
{
    const Node* srcNode = nodeFor(srcModelId, srcDepthId, srcProfileName);
    const Node* dstNode = nodeFor(dstModelId, dstDepthId, dstProfileName);
    if (srcNode == dstNode) return true;
    Path* path = findBestPath(srcNode, dstNode);
    bool existAndGood = path && path->isGood;
    delete path;
    return existAndGood;
}


QString KoColorConversionSystem::bestPathToDot(const QString& srcKey, const QString& dstKey) const
{
    const Node* srcNode = 0;
    const Node* dstNode = 0;
    foreach(Node* node, d->graph) {
        if (node->id() == srcKey) {
            srcNode = node;
        }
        if (node->id() == dstKey) {
            dstNode = node;
        }
    }
    Path* p = findBestPath(srcNode, dstNode);
    Q_ASSERT(p);
    QString dot = "digraph CCS {\n";
    dot += QString("  \"%1\" [color=red]\n").arg(srcNode->id());
    dot += QString("  \"%1\" [color=red]\n").arg(dstNode->id());
    foreach(Vertex* oV, d->vertexes) {
        QString options = "";
        if (p->vertexes.contains(oV)) {
            options = "[color=red]";
        }
        dot += vertexToDot(oV, options) ;
    }
    dot += "}\n";
    return dot;
}

void KoColorConversionSystem::deletePaths(QList<KoColorConversionSystem::Path*> paths) const
{
    foreach(Path* path, paths) {
        delete path;
    }
}

inline KoColorConversionSystem::Path* KoColorConversionSystem::findBestPathImpl2(const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode, bool ignoreHdr, bool ignoreColorCorrectness) const
{
    PathQualityChecker pQC(qMin(srcNode->referenceDepth, dstNode->referenceDepth), ignoreHdr, ignoreColorCorrectness);
    Node2PathHash node2path; // current best path to reach a given node
    QList<Path*> currentPaths; // list of all paths
    // Generate the initial list of paths
    foreach(Vertex* v, srcNode->outputVertexes) {
        if (v->dstNode->isInitialized) {
            Path* p = new Path;
            p->appendVertex(v);
            Node* endNode = p->endNode();
            if (endNode == dstNode) {
                Q_ASSERT(pQC.isGoodPath(p));  // <- it's a direct link, it has to be a good path, damn it not  or go fix your color space not
                deletePaths(currentPaths); // clean up
                p->isGood = true;
                return p;
            } else {
                Q_ASSERT(!node2path.contains(endNode));   // That would be a total fuck up if there are two vertexes between two nodes
                node2path[ endNode ] = new Path(*p);
                currentPaths.append(p);
            }
        }
    }
    Path* lessWorsePath = 0;
    // Now loop until a path has been found
    while (currentPaths.size() > 0) {
        foreach(Path* p, currentPaths) {
            Node* endNode = p->endNode();
            foreach(Vertex* v, endNode->outputVertexes) {
                if (!p->contains(v->dstNode) &&  v->dstNode->isInitialized) {
                    Path* newP = new Path(*p);
                    newP->appendVertex(v);
                    Node* newEndNode = newP->endNode();
                    if (newEndNode == dstNode) {
                        if (pQC.isGoodPath(newP)) { // Victory
                            deletePaths(currentPaths); // clean up
                            newP->isGood = true;
                            return newP;
                        } else if (!lessWorsePath) {
                            lessWorsePath = newP;
                        } else if (pQC.lessWorseThan(newP, lessWorsePath)) {
                            Q_ASSERT(newP->startNode()->id() == lessWorsePath->startNode()->id());
                            Q_ASSERT(newP->endNode()->id() == lessWorsePath->endNode()->id());
                            warnPigment << pQC.lessWorseThan(newP, lessWorsePath) << " " << newP << "  " << lessWorsePath;
                            delete lessWorsePath;
                            lessWorsePath = newP;
                        } else {
                            delete newP;
                        }
                    } else {
                        if (node2path.contains(newEndNode)) {
                            Path* p2 = node2path[newEndNode];
                            if (pQC.lessWorseThan(newP, p2)) {
                                node2path[ newEndNode ] = new Path(*newP);
                                currentPaths.append(newP);
                                delete p2;
                            } else {
                                delete newP;
                            }
                        } else {
                            node2path[ newEndNode ] = new Path(*newP);
                            currentPaths.append(newP);
                        }
                    }
                }
            }
            currentPaths.removeAll(p);
            delete p;
        }
    }
    if (lessWorsePath) {
        warnPigment << "No good path from " << srcNode->id() << " to " << dstNode->id() << " found : length = " << lessWorsePath->length() << " cost = " << lessWorsePath->cost << " referenceDepth = " << lessWorsePath->referenceDepth << " respectColorCorrectness = " << lessWorsePath->respectColorCorrectness << " isGood = " << lessWorsePath->isGood ;
        return lessWorsePath;
    }
    errorPigment << "No path from " << srcNode->id() << " to " << dstNode->id() << " found not ";
    return 0;
}

inline KoColorConversionSystem::Path* KoColorConversionSystem::findBestPathImpl(const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode, bool ignoreHdr) const
{
    Q_ASSERT(srcNode);
    Q_ASSERT(dstNode);
    if (srcNode->isGray || dstNode->isGray) {
        return findBestPathImpl2(srcNode, dstNode, ignoreHdr, true);
    } else {
        return findBestPathImpl2(srcNode, dstNode, ignoreHdr, false);
    }
}

KoColorConversionSystem::Path* KoColorConversionSystem::findBestPath(const KoColorConversionSystem::Node* srcNode, const KoColorConversionSystem::Node* dstNode) const
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
