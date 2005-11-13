/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
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
#include <qwidget.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qpixmap.h>

#include <klocale.h>

#include <kis_id.h>
#include <kis_paintop_registry.h>
#include <kis_view.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_layer.h>

#include "kis_paintop_box.h"

KisPaintopBox::KisPaintopBox (KisView * view, QWidget *parent, const char * name)
    : super (parent, name),
      m_view(view)
{
    setCaption(i18n("Painter's Toolchest"));
    m_paintops = new QValueList<KisID>();
    m_displayedOps = new QValueList<KisID>();

    connect(this, SIGNAL(selected(const KisID &)), m_view, SLOT(paintopActivated(const KisID &)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotItemSelected(int)));

    // XXX: Let's see... Are all paintops loaded and ready?
    KisIDList keys = KisPaintOpRegistry::instance()->listKeys();
    for ( KisIDList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
        // add all paintops, and show/hide them afterwards
        addItem(*it);
    }

    m_currentID = KisID("paintbrush","");

    connect(m_view, SIGNAL(currentColorSpaceChanged(KisLayerSP)),
            this, SLOT(colorSpaceChanged(KisLayerSP)));
}

KisPaintopBox::~KisPaintopBox()
{
    delete m_paintops;
    delete m_displayedOps;
}

void KisPaintopBox::addItem(const KisID & paintop, const QString & /*category*/)
{
    m_paintops->append(paintop);
}

void KisPaintopBox::slotItemSelected(int index)
{
    KisID id = *m_paintops->at(index);
    m_currentID = id;
    emit selected(id);
}

void KisPaintopBox::colorSpaceChanged(KisLayerSP layer)
{
    QValueList<KisID>::iterator it = m_paintops -> begin();
    QValueList<KisID>::iterator end = m_paintops -> end();
    KisColorSpace* cs = layer -> colorSpace();
    m_displayedOps -> clear();
    clear();

    for ( ; it != end; ++it ) {
        if (KisPaintOpRegistry::instance() -> userVisible(*it, cs)) {

            QPixmap pm = KisPaintOpRegistry::instance()->getPixmap(*it);
            if (pm.isNull()) {
                QPixmap p = QPixmap( 16, 16 );
                p.fill();
                insertItem(p,  (*it).name());
            }
            else {
                insertItem(pm, (*it).name());
            }
            m_displayedOps -> append(*it);
        }
    }

    setCurrentItem( m_displayedOps -> findIndex ( m_currentID ) );
}

#include "kis_paintop_box.moc"
