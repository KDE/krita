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

#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <koIconChooser.h>

#include "kis_itemchooser.h"
#include "kis_global.h"
#include "kis_icon_item.h"

KisItemChooser::KisItemChooser(QWidget *parent, const char *name) : super(parent)
{
    setObjectName(name);
/*    m_frame = new QVBox(this);
    m_frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);*/
    m_chooser = new KoIconChooser(QSize(30,30), this, "icon_chooser", true);
    QObject::connect(m_chooser, SIGNAL(selected(KoIconItem*)), this, SLOT(slotItemSelected(KoIconItem*)));
}

KisItemChooser::~KisItemChooser()
{
}

void KisItemChooser::setCurrent(KoIconItem *item)
{
    m_chooser->setCurrentItem(item);
    update(item);
}

void KisItemChooser::setCurrent(int index)
{
    setCurrent(m_chooser->itemAt(index));
}

KoIconItem* KisItemChooser::currentItem()
{
    return m_chooser->currentItem();
}

void KisItemChooser::slotItemSelected(KoIconItem *item)
{
    update(item);
    emit selected(currentItem());
}

void KisItemChooser::addItem(KoIconItem *item)
{
    m_chooser->addItem(item);
}

void KisItemChooser::addItems(const vKoIconItem& items)
{
    foreach (KoIconItem *item, items)
        m_chooser->addItem(item);
}

QWidget *KisItemChooser::chooserWidget() const
{
    return m_chooser;
}

#include "kis_itemchooser.moc"

