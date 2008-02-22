/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_itemchooser.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <KoResourceItemChooser.h>


#include "kis_global.h"
#include "kis_icon_item.h"

KisItemChooser::KisItemChooser(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);
/*    m_frame = new QVBox(this);
    m_frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);*/
    m_chooser = new KoResourceItemChooser(this);
    m_chooser->setMinimumSize(200, 150);

    connect( m_chooser, SIGNAL(selected(QTableWidgetItem*)), this, SLOT(slotItemSelected(QTableWidgetItem*)));
    connect( m_chooser, SIGNAL( importClicked() ), this, SIGNAL( importClicked() ) );
    connect( m_chooser, SIGNAL( deleteClicked() ), this, SIGNAL( deleteClicked() ) );
}

KisItemChooser::~KisItemChooser()
{
}

void KisItemChooser::setCurrent(QTableWidgetItem *item)
{
    m_chooser->setCurrent(item);
    update(item);
}

void KisItemChooser::setCurrent(int index)
{
    m_chooser->setCurrent(index);
}

void KisItemChooser::removeItem(KoResourceItem *item)
{
    m_chooser->removeItem(item);
}

QTableWidgetItem* KisItemChooser::currentItem()
{
    return m_chooser->currentItem();
}

void KisItemChooser::slotItemSelected(QTableWidgetItem *item)
{
    update(item);
    emit selected(currentItem());
}

void KisItemChooser::addItem(KoResourceItem *item)
{
    m_chooser->addItem(item);
}

void KisItemChooser::addItems(const QList<KoResourceItem *>& items)
{
    foreach (KoResourceItem *item, items)
        m_chooser->addItem(item);
}

QWidget *KisItemChooser::chooserWidget() const
{
    return m_chooser;
}

#include "kis_itemchooser.moc"

