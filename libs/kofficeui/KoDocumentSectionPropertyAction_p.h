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

#ifndef KO_DOCUMENT_SECTION_PROPERTY_ACTION_P_H
#define KO_DOCUMENT_SECTION_PROPERTY_ACTION_P_H

#include <QPersistentModelIndex>
#include "KoDocumentSectionModel.h"
#include "KoDocumentSectionView.h"

class KoDocumentSectionView::PropertyAction: public QAction
{
    typedef QAction super;
    Q_OBJECT
    Model::Property m_property;
    int m_num;
    QPersistentModelIndex m_index;

    signals:
        void toggled( bool on, const QPersistentModelIndex &index, int property );

    public:
        PropertyAction( int num, const Model::Property &p, const QPersistentModelIndex &index, QObject *parent = 0 )
            : QAction( parent ), m_property( p ), m_num( num ), m_index( index )
        {
            connect( this, SIGNAL( triggered( bool ) ), this, SLOT( slotTriggered() ) );
            setText( m_property.name );
            setIcon( m_property.state.toBool() ? m_property.onIcon : m_property.offIcon );
        }

    private slots:
        void slotTriggered()
        {
            m_property.state = !m_property.state.toBool();
            setIcon( m_property.state.toBool() ? m_property.onIcon : m_property.offIcon );
            emit toggled( m_property.state.toBool(), m_index, m_num );
        }
};

#endif
