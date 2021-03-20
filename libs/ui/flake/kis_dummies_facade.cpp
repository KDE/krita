/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dummies_facade.h"

#include "kis_image.h"
#include "kis_node_dummies_graph.h"


struct KisDummiesFacade::Private
{
public:
    KisNodeDummiesGraph dummiesGraph;
};

KisDummiesFacade::KisDummiesFacade(QObject *parent)
    : KisDummiesFacadeBase(parent),
      m_d(new Private())
{
}

KisDummiesFacade::~KisDummiesFacade()
{
    setImage(0);
    delete m_d;
}

void KisDummiesFacade::addNodeImpl(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    KisNodeDummy *parentDummy = parent ? dummyForNode(parent) : 0;
    KisNodeDummy *aboveThisDummy = aboveThis ? dummyForNode(aboveThis) : 0;
    KisNodeDummy *newDummy = new KisNodeDummy(0, node);

    m_d->dummiesGraph.addNode(newDummy, parentDummy, aboveThisDummy);
}

void KisDummiesFacade::removeNodeImpl(KisNodeSP node)
{
    KisNodeDummy *nodeDummy = dummyForNode(node);
    m_d->dummiesGraph.removeNode(nodeDummy);

    delete nodeDummy;
    // this dummy had no nodeShape pointer, so no cleaning here
}

bool KisDummiesFacade::hasDummyForNode(KisNodeSP node) const
{
    return m_d->dummiesGraph.containsNode(node);
}

KisNodeDummy* KisDummiesFacade::dummyForNode(KisNodeSP node) const
{
    return m_d->dummiesGraph.nodeToDummy(node);
}

KisNodeDummy* KisDummiesFacade::rootDummy() const
{
    return m_d->dummiesGraph.rootDummy();
}

int KisDummiesFacade::dummiesCount() const
{
    return m_d->dummiesGraph.dummiesCount();
}
