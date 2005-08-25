/*
 *  kis_cmb_composite.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "kis_cmb_composite.h"

KisCmbComposite::KisCmbComposite(QWidget * parent, const char * name)
    : super( false, parent, name )
{
    connect(this, SIGNAL(activated(int)), this, SLOT(slotOpActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotOpHighlighted(int)));
}

KisCmbComposite::~KisCmbComposite()
{
}

void KisCmbComposite::setCompositeOpList(const KisCompositeOpList & list)
{
    super::clear();
    m_list = list;
    KisCompositeOpList::iterator it;
        for( it = m_list.begin(); it != m_list.end(); ++it )
        insertItem((*it).id().name());
}

KisCompositeOp KisCmbComposite::currentItem() const
{
    Q_UINT32 i = super::currentItem();
    if (i > m_list.count()) return KisCompositeOp();

    return m_list[i];
}

void KisCmbComposite::setCurrentItem(const KisCompositeOp& op)
{
    if (m_list.find(op) != m_list.end()) {
        super::setCurrentText(op.id().name());
    }
}

void KisCmbComposite::setCurrentText(const QString & s)
{
    KisCompositeOpList::iterator it;
        for( it = m_list.begin(); it != m_list.end(); ++it )
        if ((*it).id().id() == s) {
            super::setCurrentText((*it).id().name());
        }
}

void KisCmbComposite::slotOpActivated(int i)
{
    if ((Q_UINT32)i > m_list.count()) return;

    emit activated(m_list[i]);
}

void KisCmbComposite::slotOpHighlighted(int i)
{
    if ((Q_UINT32)i > m_list.count()) return;

    emit highlighted(m_list[i]);
}


#include "kis_cmb_composite.moc"

