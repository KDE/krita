/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

 */


#include <QDockWidget>
#include <QToolBox>

#include "kopalette.h"
#include "kopalettemanager.h"
#include "kotoolboxpalette.h"

KoToolBoxPalette::KoToolBoxPalette(QWidget * parent, const char * name)
    : KoPalette(parent, name)
{
    m_page = new QToolBox(this);
    m_page->unsetFont();
    setMainWidget(m_page);
    m_style = PALETTE_TOOLBOX;
}

KoToolBoxPalette::~KoToolBoxPalette()
{
}


void KoToolBoxPalette::resetFont()
{
    KoPalette::resetFont();
    m_page->unsetFont();
}


void KoToolBoxPalette::plug(QWidget *w, const QString & label, int position)
{
    w->unsetFont();
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

void KoToolBoxPalette::hidePage( QWidget * w)
{
    if (m_hiddenPages.find(w) != m_hiddenPages.end()) return;
    int i = m_page->indexOf(w);
    m_page->removeItem(w);
    m_hiddenPages[w] = i;
    if (m_page->count() == 0) {
        hide();
    }
}

#include "kotoolboxpalette.moc"
