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

#include "widgets/kis_cmb_idlist.h"

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <KoID.h>

KisCmbIDList::KisCmbIDList(QWidget * parent, const char * name)
    : QComboBox(parent)
{
    setObjectName(name);
    setEditable(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotIDActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotIDHighlighted(int)));
}

KisCmbIDList::~KisCmbIDList()
{
}


void KisCmbIDList::setIDList(const QList<KoID>  & list, bool sorted)
{
    clear();
    m_list = list;
    if (sorted) {
        std::sort(m_list.begin(), m_list.end(), KoID::compareNames);
    }
    for (qint32 i = 0; i < m_list.count(); ++i) {
        addItem(m_list.at(i).name());
    }
}


KoID KisCmbIDList::currentItem() const
{
    qint32 i = QComboBox::currentIndex();
    if (i > m_list.count() - 1 || i < 0) return KoID();

    return m_list[i];
}

void KisCmbIDList::setCurrent(const KoID id)
{
    qint32 index = m_list.indexOf(id);

    if (index >= 0) {
        QComboBox::setCurrentIndex(index);
    } else {
        m_list.push_back(id);
        addItem(id.name());
        QComboBox::setCurrentIndex(m_list.count() - 1);
    }
}

void KisCmbIDList::setCurrent(const QString & s)
{
    for (qint32 i = 0; i < m_list.count(); ++i) {
        if (m_list.at(i).id() == s) {
            QComboBox::setCurrentIndex(i);
            break;
        }
    }
}

void KisCmbIDList::slotIDActivated(int i)
{
    if (i > m_list.count() - 1) return;

    emit activated(m_list[i]);

}

void KisCmbIDList::slotIDHighlighted(int i)
{
    if (i > m_list.count() - 1) return;

    emit highlighted(m_list[i]);

}


