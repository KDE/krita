/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NODE_PROPERTY_ACTION_P_H
#define NODE_PROPERTY_ACTION_P_H

#include <kis_node_model.h>
#include <kis_base_node.h>

#include <QPersistentModelIndex>
#include <QAction>

#include "NodeView.h"
/**
 * Internal class for the NodeView widget. Provides a
 * toggle action for a particular property associated with a document
 * section, such as visible, selected, locked. Property actions have
 * associated on/off icons to show their state in the
 * NodeView.
 */
class NodeView::PropertyAction: public QAction
{
    typedef QAction super;
    Q_OBJECT
    KisBaseNode::Property m_property;
    int m_num;
    QPersistentModelIndex m_index;

    Q_SIGNALS:
        void toggled( bool on, const QPersistentModelIndex &index, int property );

    public:
        PropertyAction( int num, const KisBaseNode::Property &p, const QPersistentModelIndex &index, QObject *parent = 0 )
            : QAction( parent ), m_property( p ), m_num( num ), m_index( index )
        {
            connect( this, SIGNAL( triggered( bool ) ), this, SLOT( slotTriggered() ) );
            setText( m_property.name );
            setIcon( m_property.state.toBool() ? m_property.onIcon : m_property.offIcon );
        }

    private Q_SLOTS:
        void slotTriggered()
        {
            m_property.state = !m_property.state.toBool();
            setIcon( m_property.state.toBool() ? m_property.onIcon : m_property.offIcon );
            emit toggled( m_property.state.toBool(), m_index, m_num );
        }
};

#endif
