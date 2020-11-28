/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_NODE_COMMANDS_ADAPTER_H
#define KIS_NODE_COMMANDS_ADAPTER_H

class KisViewManager;
class KoCompositeOp;
class KUndo2MagicString;
class KisProcessingApplicator;


#include <kis_types.h>
#include <kritaui_export.h>

#include <QObject>

/**
 * This class allows the manipulation of nodes in a KisImage
 * and creates commands as needed.
 */
class KRITAUI_EXPORT KisNodeCommandsAdapter : public QObject
{
    Q_OBJECT

public:
    KisNodeCommandsAdapter(KisViewManager * view);
    ~KisNodeCommandsAdapter() override;
public:
    /**
     * Applies \p cmd on a provided \p applicator. If \p applicator is null, then a temporary
     * applicator is created locally.
     */
    void applyOneCommandAsync(KUndo2Command *cmd, KisProcessingApplicator *applicator = 0);

    /**
     * Same as addNode(), but adds a node using the provided \p applicator in an asynchronous way.
     * If \p applicator is null, then a temporary applicator (with a stroke) is created.
     */
    void addNodeAsync(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis, bool doRedoUpdates = true, bool doUndoUpdates = true, KisProcessingApplicator *applicator = 0);
    void addNodeAsync(KisNodeSP node, KisNodeSP parent, quint32 index, bool doRedoUpdates = true, bool doUndoUpdates = true, KisProcessingApplicator *applicator = 0);


    void beginMacro(const KUndo2MagicString& macroName);
    void addExtraCommand(KUndo2Command *command);
    void endMacro();
    void addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis, bool doRedoUpdates = true, bool doUndoUpdates = true);
    void addNode(KisNodeSP node, KisNodeSP parent, quint32 index, bool doRedoUpdates = true, bool doUndoUpdates = true);
    void moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void moveNode(KisNodeSP node, KisNodeSP parent, quint32 indexaboveThis);
    void removeNode(KisNodeSP node);
    void setOpacity(KisNodeSP node, qint32 opacity);
    void setCompositeOp(KisNodeSP node, const KoCompositeOp* compositeOp);
    void setNodeName(KisNodeSP node, const QString &name);

    void undoLastCommand();
private:
    KisViewManager* m_view;
};

#endif // KIS_NODE_COMMANDS_ADAPTER_H
