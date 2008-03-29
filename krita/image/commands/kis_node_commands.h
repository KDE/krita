/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
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
#ifndef KIS_NODE_COMMANDS_H_
#define KIS_NODE_COMMANDS_H_

#include <krita_export.h>
#include <QUndoCommand>
#include <QRect>
#include "kis_types.h"
#include <klocale.h>
#include "filter/kis_filter_configuration.h"

class KisNode;

template <typename T>
class KisChangeFilterCmd : public QUndoCommand {

public:
    // The QStrings are the _serialized_ configs
    KisChangeFilterCmd(T node,
                       KisFilterConfiguration* config,
                       const QString& before,
                       const QString& after)
        : QUndoCommand(i18n("Change Filter"))
    {
        m_node = node;
        m_config = config;
        m_before = before;
        m_after = after;
    }
public:
    virtual void redo()
    {
        m_config->fromLegacyXML(m_after);
        m_node->setFilter(m_config);
        m_node->setDirty();
    }

    virtual void undo()
    {
        m_config->fromLegacyXML(m_before);
        m_node->setFilter(m_config);
        m_node->setDirty();
    }

private:
    T m_node;
    KisFilterConfiguration* m_config;
    QString m_before;
    QString m_after;
};


/// The command for moving of a node
class KRITAIMAGE_EXPORT KisNodeMoveCommand : public QUndoCommand {

public:
    /**
     * Constructor
     * @param node The node the command will be working on.
     * @param oldpos the old layer position
     * @param newpos the new layer position
     */
    KisNodeMoveCommand(KisNodeSP node, const QPoint& oldpos, const QPoint& newpos);
    virtual ~KisNodeMoveCommand();

    virtual void redo();
    virtual void undo();

private:
    void moveTo(const QPoint& pos);

private:
    KisNodeSP m_node;
    QRect m_updateRect;
    QPoint m_oldPos;
    QPoint m_newPos;
};


#endif // KIS_NODE_COMMANDS_H_
