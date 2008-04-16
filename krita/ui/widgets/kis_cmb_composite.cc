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

#include "widgets/kis_cmb_composite.h"

#include <klocale.h>
#include <kis_debug.h>
#include <KoCompositeOp.h>


static QStringList opsInOrder;

KisCmbComposite::KisCmbComposite(QWidget * parent, const char * name)
    : KComboBox(parent)
{
    setObjectName(name);
    setEditable(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotOpActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotOpHighlighted(int)));
    if (opsInOrder.isEmpty()) {
        opsInOrder <<
            COMPOSITE_OVER <<
            COMPOSITE_ERASE <<
            COMPOSITE_COPY <<
            COMPOSITE_ALPHA_DARKEN <<
            COMPOSITE_IN <<
            COMPOSITE_OUT <<
            COMPOSITE_ATOP <<
            COMPOSITE_XOR <<
            COMPOSITE_PLUS <<
            COMPOSITE_MINUS <<
            COMPOSITE_ADD <<
            COMPOSITE_SUBTRACT <<
            COMPOSITE_DIFF <<
            COMPOSITE_MULT <<
            COMPOSITE_DIVIDE <<
            COMPOSITE_DODGE <<
            COMPOSITE_BURN <<
            COMPOSITE_BUMPMAP <<
            COMPOSITE_CLEAR <<
            COMPOSITE_DISSOLVE <<
            COMPOSITE_DISPLACE <<
            COMPOSITE_NO <<
            COMPOSITE_DARKEN <<
            COMPOSITE_LIGHTEN <<
            COMPOSITE_HUE <<
            COMPOSITE_SATURATION <<
            COMPOSITE_VALUE <<
            COMPOSITE_COLOR <<
            COMPOSITE_COLORIZE <<
            COMPOSITE_LUMINIZE <<
            COMPOSITE_SCREEN <<
            COMPOSITE_OVERLAY <<
            COMPOSITE_UNDEF <<
            COMPOSITE_COPY_RED <<
            COMPOSITE_COPY_GREEN <<
            COMPOSITE_COPY_BLUE <<
            COMPOSITE_COPY_OPACITY;
    }
}

KisCmbComposite::~KisCmbComposite()
{
}

void KisCmbComposite::setCompositeOpList(const QList<KoCompositeOp*> & list)
{
    KComboBox::clear();
    m_list.clear();

    // First, insert all composite ops that we know about, in a nice order.
    foreach (QString id, opsInOrder) {
        foreach (KoCompositeOp* op, list) {
            if ( op->id() == id ) {
                m_list.append(op);
                addItem(op->description());
            }
        }
    }
    // Then check for all given composite ops whether they are already inserted, and if not, add them at the end.
    foreach(KoCompositeOp* op1, list) {
        bool found = false; 
        foreach(KoCompositeOp* op2, m_list) {
            if (op1->id() == op2->id()) {
                found = true;
            }
        }
        if (!found && op1->userVisible()) {
            m_list.append(op1);
            addItem(op1->description());
        }
    }
}

KoCompositeOp * KisCmbComposite::currentItem() const
{
    qint32 i = KComboBox::currentIndex();
    if (i > m_list.count() - 1) return 0;

    return m_list[i];
}

void KisCmbComposite::setCurrent(const KoCompositeOp* op)
{
    int index = 0;
    foreach( KoCompositeOp * op2, m_list) {
        if (op->id() == op2->id())
            break;
        ++index;
    }
    if (index >= 0 && index < m_list.size() ) {
        KComboBox::setCurrentIndex(index);
    }
}

void KisCmbComposite::setCurrent(const QString & s)
{
    for (int i = 0; i < m_list.count(); ++i) {
        if (m_list.at(i)->id() == s) {
            KComboBox::setCurrentIndex(i);
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

