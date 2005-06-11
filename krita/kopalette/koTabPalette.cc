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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <qdockwindow.h>

#include <ktabwidget.h>

#include <koView.h>

#include <koPalette.h>
#include <koTabPalette.h>

KoTabPalette::KoTabPalette(KoView * parent, const char * name)
	: KoPalette(parent, name)
{
	m_page = new KTabWidget(this);
	m_page->setTabShape(QTabWidget::Triangular);
	m_page->setFont(m_font);
	setMainWidget(m_page);
}

KoTabPalette::~KoTabPalette()
{
}

void KoTabPalette::plug(QWidget * w, const QString & /*name*/)
{
	w -> setFont(m_font);
        m_page -> addTab(w, w -> caption());
	if (w -> layout() != 0) {
 		w -> layout() -> setSpacing(0);
 		w -> layout() -> setMargin(0);
	}

}

void KoTabPalette::unplug(const QWidget * w)
{
	m_page->removePage(const_cast<QWidget *>(w));
}

void KoTabPalette::showPage(QWidget *w)
{
	m_page->showPage(w);
}

#include "koTabPalette.moc"
