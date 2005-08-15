/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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


#include <qdockwindow.h>
#include <qtoolbox.h>

#include <kopalette.h>
#include <kopalettemanager.h>
#include <kotoolboxpalette.h>

KoToolBoxPalette::KoToolBoxPalette(KoView * parent, const char * name)
    : KoPalette(parent, name)
{
    m_page = new QToolBox(this);
    m_page->setFont(m_font);
    setMainWidget(m_page);
}

KoToolBoxPalette::~KoToolBoxPalette()
{
}

void KoToolBoxPalette::plug(QWidget *w, const QString & label, int position)
{
    w->setFont(m_font);
    m_page->insertItem( position, w,  label );
}


void KoToolBoxPalette::unplug(const QWidget *w)
{
    m_page->removeItem( const_cast<QWidget*>(w) );
}

void KoToolBoxPalette::showPage(QWidget *w)
{
    m_page->setCurrentItem( w );
}


int KoToolBoxPalette::indexOf(QWidget *w)
{
    if (m_hiddenPages.find(w) != m_hiddenPages.end()) {
        return m_page->indexOf(w);
    }
    else {
        return m_page->indexOf(w);
    }
}


void KoToolBoxPalette::makeVisible(bool v)
{
    if (v && m_page->count() > 0) {
        show();
    }
    else {
        hide();
    }
}

bool KoToolBoxPalette::isHidden(QWidget * w)
{
    return (m_hiddenPages.find(w) != m_hiddenPages.end());
}

void KoToolBoxPalette::togglePageHidden(QWidget *w)
{
    if (m_hiddenPages.find(w) != m_hiddenPages.end()) {
        int i = *m_hiddenPages.find(w);
        m_page->insertItem(i, w, w->caption());
        show();
    }
    else {
        int i = m_page->indexOf(w);
        m_page->removeItem(w);
        m_hiddenPages[w] = i;
        if (m_page->count() == 0) {
            hide();
        }
    } 

}

#include "kotoolboxpalette.moc"
