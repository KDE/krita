/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dummies_facade_base.h"

#include "kis_image.h"
#include "kis_node_dummies_graph.h"
#include "kis_layer_utils.h"
#include <KisSynchronizedConnection.h>

struct KisDummiesFacadeBase::Private
{
public:
    KisImageWSP image;
    KisNodeSP savedRootNode;

    KisSynchronizedConnection<KisNodeSP> activateNodeConnection;
    KisSynchronizedConnection<KisNodeSP> nodeChangedConnection;
    KisSynchronizedConnection<KisNodeSP,KisNodeSP,KisNodeSP> addNodeConnection;
    KisSynchronizedConnection<KisNodeSP> removeNodeConnection;
};


KisDummiesFacadeBase::KisDummiesFacadeBase(QObject *parent)
    : QObject(parent),
      m_d(new Private())
{
    m_d->activateNodeConnection.connectOutputSlot(this, &KisDummiesFacadeBase::slotNodeActivationRequested);
    m_d->nodeChangedConnection.connectOutputSlot(this, &KisDummiesFacadeBase::slotNodeChanged);
    m_d->addNodeConnection.connectOutputSlot(this, &KisDummiesFacadeBase::slotContinueAddNode);
    m_d->removeNodeConnection.connectOutputSlot(this, &KisDummiesFacadeBase::slotContinueRemoveNode);
}

KisDummiesFacadeBase::~KisDummiesFacadeBase()
{
    delete m_d;
}

void KisDummiesFacadeBase::setImage(KisImageWSP image)
{
    if (m_d->image) {
        emit sigActivateNode(0);
        m_d->image->disconnect(this);
        m_d->nodeChangedConnection.disconnectInputSignals();
        m_d->activateNodeConnection.disconnectInputSignals();

        if (rootDummy()) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->addNodeConnection.hasPendingSignals());
            KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->removeNodeConnection.hasPendingSignals());

            KisNodeList nodesToRemove;

            KisLayerUtils::recursiveApplyNodes(rootDummy(),
                [&nodesToRemove] (KisNodeDummy *dummy) {
                    nodesToRemove << dummy->node();
                });

            for (auto it = std::make_reverse_iterator(nodesToRemove.end());
                 it != std::make_reverse_iterator(nodesToRemove.begin());
                 ++it) {

                m_d->removeNodeConnection.start(*it);
            }
        }
    }

    m_d->image = image;

    if (image) {
        slotNodeAdded(image->root());

        connect(image, SIGNAL(sigNodeAddedAsync(KisNodeSP)),
                SLOT(slotNodeAdded(KisNodeSP)), Qt::DirectConnection);
        connect(image, SIGNAL(sigRemoveNodeAsync(KisNodeSP)),
                SLOT(slotRemoveNode(KisNodeSP)), Qt::DirectConnection);
        connect(image, SIGNAL(sigLayersChangedAsync()),
                SLOT(slotLayersChanged()), Qt::DirectConnection);

        m_d->nodeChangedConnection.connectInputSignal(image, &KisImage::sigNodeChanged);
        m_d->activateNodeConnection.connectInputSignal(image, &KisImage::sigNodeAddedAsync);

        m_d->activateNodeConnection.start(findFirstLayer(image->root()));
    }
}

KisImageWSP KisDummiesFacadeBase::image() const
{
    return m_d->image;
}

KisNodeSP KisDummiesFacadeBase::findFirstLayer(KisNodeSP root)
{
    KisNodeSP child = root->firstChild();
    while(child && !child->inherits("KisLayer")) {
        child = child->nextSibling();
    }
    return child;
}

void KisDummiesFacadeBase::slotNodeChanged(KisNodeSP node)
{
    KisNodeDummy *dummy = dummyForNode(node);

    /**
     * In some "buggy" code the node-changed signal may be emitted
     * before the node will become a part of the node graph. It is
     * a bug, we a really minor one. It should not cause any data
     * losses to the user.
     */
    KIS_SAFE_ASSERT_RECOVER_RETURN(dummy);

    emit sigDummyChanged(dummy);
}

void KisDummiesFacadeBase::slotLayersChanged()
{
    setImage(m_d->image);
}

void KisDummiesFacadeBase::slotNodeActivationRequested(KisNodeSP node)
{
    if (!node || !node->graphListener()) return;

    if (!node->inherits("KisSelectionMask") &&
        !node->inherits("KisReferenceImagesLayer") &&
        !node->inherits("KisDecorationsWrapperLayer")) {

        emit sigActivateNode(node);
    }
}

void KisDummiesFacadeBase::slotNodeAdded(KisNodeSP node)
{
    m_d->addNodeConnection.start(node, node->parent(), node->prevSibling());

    KisNodeSP childNode = node->firstChild();
    while (childNode) {
        slotNodeAdded(childNode);
        childNode = childNode->nextSibling();
    }
}

void KisDummiesFacadeBase::slotRemoveNode(KisNodeSP node)
{
    KisNodeSP childNode = node->lastChild();
    while (childNode) {
        slotRemoveNode(childNode);
        childNode = childNode->prevSibling();
    }

    m_d->removeNodeConnection.start(node);
}

void KisDummiesFacadeBase::slotContinueAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    KisNodeDummy *parentDummy = parent ? dummyForNode(parent) : 0;
    KisNodeDummy *aboveThisDummy = aboveThis ? dummyForNode(aboveThis) : 0;
    // Add one because this node does not exist yet
    int index = parentDummy && aboveThisDummy ?
        parentDummy->indexOf(aboveThisDummy) + 1 : 0;
    emit sigBeginInsertDummy(parentDummy, index, node->metaObject()->className());

    addNodeImpl(node, parent, aboveThis);

    emit sigEndInsertDummy(dummyForNode(node));
}

void KisDummiesFacadeBase::slotContinueRemoveNode(KisNodeSP node)
{
    KisNodeDummy *dummy = dummyForNode(node);
    emit sigBeginRemoveDummy(dummy);

    removeNodeImpl(node);

    emit sigEndRemoveDummy();
}
