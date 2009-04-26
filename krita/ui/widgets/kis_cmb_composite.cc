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
#include "../kis_composite_ops_model.h"

static QStringList opsInOrder;

KisCmbComposite::KisCmbComposite(QWidget * parent, const char * name)
        : KComboBox(parent), m_lastModel(0)
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
        COMPOSITE_SUBSTRACT <<
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
    KisCompositeOpsModel* model = new KisCompositeOpsModel(list);
    setModel(model);
    
    delete m_lastModel;
    m_lastModel = model;
    
#if 0
    KComboBox::clear();
    m_list.clear();

    // First, insert all composite ops that we know about, in a nice order.
    foreach(const QString & id, opsInOrder) {
        foreach(KoCompositeOp* op, list) {
            if (op->id() == id) {
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
#endif
}

KoCompositeOp * KisCmbComposite::currentItem() const
{
    return m_lastModel->itemAt(currentIndex());
}

void KisCmbComposite::setCurrent(const KoCompositeOp* op)
{
    QModelIndex index = m_lastModel->indexOf(op);
    if (index.isValid()) {
        KComboBox::setCurrentIndex(index.row());
    }
}

void KisCmbComposite::setCurrent(const QString & s)
{
    QModelIndex index = m_lastModel->indexOf(s);
    if (index.isValid()) {
        KComboBox::setCurrentIndex(index.row());
    }
}

void KisCmbComposite::slotOpActivated(int i)
{
    if (i >= m_lastModel->rowCount()) return;

    emit activated(m_lastModel->itemAt(i));
}

void KisCmbComposite::slotOpHighlighted(int i)
{
    if (i >= m_lastModel->rowCount()) return;

    emit activated(m_lastModel->itemAt(i));
}


#include "kis_cmb_composite.moc"

