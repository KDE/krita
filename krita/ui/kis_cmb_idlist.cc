/*
 *  kis_cmb_idlist.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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

#include <qcombobox.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_id.h"
#include "kis_cmb_idlist.h"

KisCmbIDList::KisCmbIDList(QWidget * parent, const char * name)
    : super( false, parent, name )
{
    connect(this, SIGNAL(activated(int)), this, SLOT(slotIDActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotIDHighlighted(int)));
}

KisCmbIDList::~KisCmbIDList()
{
}


void KisCmbIDList::setIDList(const KisIDList & list)
{
    m_list = list;
    KisIDList::iterator it;
        for( it = m_list.begin(); it != m_list.end(); ++it )
        insertItem((*it).name());
}


KisID KisCmbIDList::currentItem() const
{
    Q_UINT32 i = super::currentItem();
    if (i > m_list.count()) return KisID();

    return m_list[i];
}

void KisCmbIDList::setCurrent(const KisID id)
{
    if (m_list.find(id) != m_list.end()) 
        super::setCurrentText(id.name());
    else {
        m_list.push_back(id);
        insertItem(id.name());
        super::setCurrentText(id.name());
    }
}

void KisCmbIDList::setCurrentText(const QString & s)
{
    KisIDList::iterator it;
        for( it = m_list.begin(); it != m_list.end(); ++it )
        if ((*it).id() == s) {
            super::setCurrentText((*it).name());
        }
}

void KisCmbIDList::slotIDActivated(int i)
{
    if ((uint)i > m_list.count()) return;

    emit activated(m_list[i]);

}

void KisCmbIDList::slotIDHighlighted(int i)
{
    if ((uint)i > m_list.count()) return;

    emit highlighted(m_list[i]);

}



#include "kis_cmb_idlist.moc"

