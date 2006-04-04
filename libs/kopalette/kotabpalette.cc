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
   the Free Software Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <QDockWidget>

#include <ktabwidget.h>
#include <kdebug.h>

#include "KoView.h"

#include "kopalette.h"
#include "kotabpalette.h"
#include "kopalettemanager.h"

KoTabPalette::KoTabPalette(QWidget * parent, const char * name)
    : KoPalette(parent, name)
{
    m_page = new KTabWidget(this);
    m_page->setTabShape(QTabWidget::Triangular);
    m_page->setFocusPolicy(Qt::TabFocus);
    //m_page->setFont(m_font);
    m_page->setMargin(1);
    //m_page->setTabReorderingEnabled(true);
    setMainWidget(m_page);
    m_style = PALETTE_DOCKER;
}

KoTabPalette::~KoTabPalette()
{
}

void KoTabPalette::plug(QWidget * w, const QString & /*name*/, int position)
{
    if (!w) return;
    
    w -> unsetFont(); // Use the parent font

    m_page -> insertTab(w, w -> caption(), position);
    show();
}

void KoTabPalette::resetFont() 
{
    KoPalette::resetFont();
    m_page->unsetFont();
}

void KoTabPalette::unplug(const QWidget * w)
{
    m_page->removePage(const_cast<QWidget *>(w));
    if (m_page->count() == 0)
        hide();
}

void KoTabPalette::showPage(QWidget *w)
{
    m_page->showPage(w);
    if (m_hiddenPages.find(w) == m_hiddenPages.end()) return;

    int i = *m_hiddenPages.find(w);
    m_page->insertTab(w, w->caption(), i);
    m_hiddenPages.erase(w);
    show();
}

void KoTabPalette::makeVisible(bool v)
{
    //kDebug() << "make visible called for " << name() << ", " << v << "\n";
    if (v && m_page->count() > 0)
        show();
    else
        hide();
}

int KoTabPalette::indexOf(QWidget *w)
{
    if (m_hiddenPages.find(w) != m_hiddenPages.end()) {
        int i = m_page->indexOf(w);
        return -i;
    }
    else {
        return m_page->indexOf(w);
    }
}

bool KoTabPalette::isHidden(QWidget * w)
{
    return (m_hiddenPages.find(w) != m_hiddenPages.end());
}

void KoTabPalette::hidePage( QWidget *w)
{
    if (m_hiddenPages.find(w) != m_hiddenPages.end()) return;
    
    int i = m_page->indexOf(w);
    m_page->removePage(w);
    m_hiddenPages[w] = i;
    if (m_page->count() == 0) {
        hide();
    }
}

void KoTabPalette::togglePageHidden(QWidget *w)
{
    if (m_hiddenPages.find(w) != m_hiddenPages.end()) {
        int i = *m_hiddenPages.find(w);
        m_page->insertTab(w, w->caption(), i);
        m_hiddenPages.erase(w);
        show();
    }
    else {
        int i = m_page->indexOf(w);
        m_page->removePage(w);
        m_hiddenPages[w] = i;
        if (m_page->count() == 0) {
            hide();
        }
    }
}

#include "kotabpalette.moc"
