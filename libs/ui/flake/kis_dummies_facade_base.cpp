/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dummies_facade_base.h"

#include "kis_image.h"
#include "kis_node_dummies_graph.h"


struct KisDummiesFacadeBase::Private
{
public:
    KisImageWSP image;
    KisNodeSP savedRootNode;
};


KisDummiesFacadeBase::KisDummiesFacadeBase(QObject *parent)
    : QObject(parent),
      m_d(new Private())
{
    connect(this, SIGNAL(sigContinueAddNode(KisNodeSP,KisNodeSP,KisNodeSP)),
            SLOT(slotContinueAddNode(KisNodeSP,KisNodeSP,KisNodeSP)));
    connect(this, SIGNAL(sigContinueRemoveNode(KisNodeSP)),
            SLOT(slotContinueRemoveNode(KisNodeSP)));
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

        KisNodeDummy *rootDummy = this->rootDummy();
        if(rootDummy) {
            slotRemoveNode(rootDummy->node());
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

        connect(image, SIGNAL(sigNodeChanged(KisNodeSP)),
                SLOT(slotNodeChanged(KisNodeSP)));

        connect(image, SIGNAL(sigNodeAddedAsync(KisNodeSP)),
                SLOT(slotNodeActivationRequested(KisNodeSP)), Qt::AutoConnection);
        emit sigActivateNode(findFirstLayer(image->root()));
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
    if (!node->graphListener()) return;

    if (!node->inherits("KisSelectionMask") && !node->inherits("KisReferenceImagesLayer")) {
        emit sigActivateNode(node);
    }
}

void KisDummiesFacadeBase::slotNodeAdded(KisNodeSP node)
{
    emit sigContinueAddNode(node, node->parent(), node->prevSibling());

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

    emit sigContinueRemoveNode(node);
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
