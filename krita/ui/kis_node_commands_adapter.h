/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_NODE_COMMANDS_ADAPTER_H
#define KIS_NODE_COMMANDS_ADAPTER_H

class KisView2;
class KoCompositeOp;

#include <kis_types.h>
#include <krita_export.h>

#include <QObject>

/**
 * This class allows the manipulation of nodes in a KisImage
 * and creates commands as needed.
 */
class KRITAUI_EXPORT KisNodeCommandsAdapter : public QObject
{
    Q_OBJECT

public:
    KisNodeCommandsAdapter(KisView2 * view);
    virtual ~KisNodeCommandsAdapter();
public:
    void beginMacro(const QString& macroName);
    void endMacro();
    void addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void addNode(KisNodeSP node, KisNodeSP parent, quint32 index);
    void moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void moveNode(KisNodeSP node, KisNodeSP parent, quint32 indexaboveThis);
    void removeNode(KisNodeSP node);
    void lower(KisNodeSP node);
    void raise(KisNodeSP node);
    void toBottom(KisNodeSP node);
    void toTop(KisNodeSP node);
    void setOpacity(KisNodeSP node, qint32 opacity);
    void setCompositeOp(KisNodeSP node, const KoCompositeOp* compositeOp);

    void undoLastCommand();
private:
    KisView2* m_view;
};

#endif // KIS_NODE_COMMANDS_ADAPTER_H
