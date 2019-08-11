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

#ifndef __KIS_DUMMIES_FACADE_BASE_H
#define __KIS_DUMMIES_FACADE_BASE_H

#include <QObject>

#include "kis_types.h"
#include "kritaui_export.h"

class KisNodeDummy;

/**
 * Keeps track of the node stack and manages local (UI-wide) representation
 * of the node stack. It uses KisNodeDummy objects to represent the stack.
 * This is done to break synchronization tie between UI and Image threads,
 * caused by the fact that KisNodeModel must be synchronously notified
 * when a node is removed/deleted.
 */

class KRITAUI_EXPORT KisDummiesFacadeBase : public QObject
{
    Q_OBJECT

public:
    KisDummiesFacadeBase(QObject *parent = 0);
    ~KisDummiesFacadeBase() override;

    virtual void setImage(KisImageWSP image);

    virtual bool hasDummyForNode(KisNodeSP node) const = 0;
    virtual KisNodeDummy* dummyForNode(KisNodeSP node) const = 0;
    virtual KisNodeDummy* rootDummy() const = 0;
    virtual int dummiesCount() const = 0;

protected:
    KisImageWSP image() const;

    virtual void addNodeImpl(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis) = 0;
    virtual void removeNodeImpl(KisNodeSP node) = 0;

Q_SIGNALS:

    void sigContinueAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void sigContinueRemoveNode(KisNodeSP node);

    /**
     * The signals for controlling the node model
     */

    void sigBeginInsertDummy(KisNodeDummy *parent, int index, const QString &metaObjectType);
    void sigEndInsertDummy(KisNodeDummy *dummy);

    void sigBeginRemoveDummy(KisNodeDummy *dummy);
    void sigEndRemoveDummy();

    void sigDummyChanged(KisNodeDummy *dummy);

    /**
     * This signal is emitted when the shape controller wants to request
     * the change of an active layer. E.g. when a new layer is added or
     * when the root layer of the image is changed. It should be forwarded
     * through a signal to allow queueing and synchronization of threads.
     */
    void sigActivateNode(KisNodeSP node);

private Q_SLOTS:
    void slotLayersChanged();
    void slotNodeChanged(KisNodeSP node);

    void slotNodeActivationRequested(KisNodeSP node);

    void slotNodeAdded(KisNodeSP node);
    void slotRemoveNode(KisNodeSP node);

    void slotContinueAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void slotContinueRemoveNode(KisNodeSP node);

private:
    static KisNodeSP findFirstLayer(KisNodeSP root);

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_DUMMIES_FACADE_BASE_H */
