/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_JUGGLER_COMPRESSED_H
#define __KIS_NODE_JUGGLER_COMPRESSED_H

#include <QObject>
#include <QScopedPointer>

#include <kritaui_export.h>
#include <kundo2command.h>
#include "kis_types.h"
#include "kis_node_manager.h"


class KRITAUI_EXPORT KisNodeJugglerCompressed : public QObject
{
    Q_OBJECT
public:
    KisNodeJugglerCompressed(const KUndo2MagicString &actionName, KisImageSP image, KisNodeManager *nodeManager, int timeout);
    ~KisNodeJugglerCompressed() override;

    void moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP above);
    void setAutoDelete(bool value);

    bool isEnded() const;

    void lowerNode(const KisNodeList &nodes);
    void raiseNode(const KisNodeList &nodes);
    void removeNode(const KisNodeList &nodes);
    void duplicateNode(const KisNodeList &nodes);

    void copyNode(const KisNodeList &nodes, KisNodeSP dstParent, KisNodeSP dstAbove);
    void moveNode(const KisNodeList &nodes, KisNodeSP dstParent, KisNodeSP dstAbove);
    void addNode(const KisNodeList &nodes, KisNodeSP dstParent, KisNodeSP dstAbove);

    bool canMergeAction(const KUndo2MagicString &actionName);

public Q_SLOTS:
    void end();

private Q_SLOTS:
    void startTimers();
    void slotUpdateTimeout();
    void slotEndStrokeRequested();
    void slotCancelStrokeRequested();
    void slotImageAboutToBeDeleted();

Q_SIGNALS:
    void requestUpdateAsyncFromCommand();

private:
    void cleanup();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_NODE_JUGGLER_COMPRESSED_H */
