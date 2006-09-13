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

#include <QComboBox>

#include <klocale.h>
#include <kdebug.h>
#include <KoCompositeOp.h>
#include "kis_cmb_composite.h"

KisCmbComposite::KisCmbComposite(QWidget * parent, const char * name)
    : super(parent)
{
    setObjectName(name);
    setEditable(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotOpActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotOpHighlighted(int)));
}

KisCmbComposite::~KisCmbComposite()
{
}

void KisCmbComposite::setCompositeOpList(const KoCompositeOpList & list)
{
    super::clear();
    m_list = list;

    for(int i = 0; i < m_list.count(); ++i)
        addItem(m_list.at(i)->description());
}

KoCompositeOp * KisCmbComposite::currentItem() const
{
    qint32 i = super::currentIndex();
    if (i > m_list.count() - 1) return 0;

    return m_list[i];
}

void KisCmbComposite::setCurrent(const KoCompositeOp* op)
{
    qint32 index = m_list.indexOf(const_cast<KoCompositeOp*>( op ));

    if (index >= 0) {
        super::setCurrentIndex(index);
    }
}

void KisCmbComposite::setCurrent(const QString & s)
{
    for (int i = 0; i < m_list.count(); ++i) {
        if (m_list.at(i)->id() == s) {
            super::setCurrentIndex(i);
            break;
        }
    }
}

void KisCmbComposite::slotOpActivated(int i)
{
    if (i > m_list.count() - 1) return;

    emit activated(m_list[i]);
}

void KisCmbComposite::slotOpHighlighted(int i)
{
    if (i > m_list.count() - 1) return;

    emit highlighted(m_list[i]);
}


#include "kis_cmb_composite.moc"

