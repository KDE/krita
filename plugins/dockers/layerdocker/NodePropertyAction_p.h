/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
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
