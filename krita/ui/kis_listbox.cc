/*
 *  kis_listbox.cc - part of Krita aka Krayon aka KimageShop
 *
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qbrush.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qstring.h>
#include <qstyle.h>
#include <qwidget.h>

#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kis_listbox.h"

const int HEIGHT = 32;

KisListBoxItem::KisListBoxItem(const QString& name, QListBox *parent, int flags) : super(parent)
{
	KIconLoader il;

	m_name = name;
	m_visiblePix = loadPixmap("visible.png", il);
	m_visibleRect = QRect(QPoint(3, (HEIGHT - 24) / 2), QSize(24,24));
	m_invisiblePix = loadPixmap("novisible.png", il);
	m_linkedPix = loadPixmap("linked.png", il);
	m_linkedRect = QRect(QPoint(30, (HEIGHT - 24) / 2), QSize(24,24));
	m_unlinkedPix = loadPixmap("unlinked.png", il);
	m_previewRect = QRect(QPoint(57, (HEIGHT - 24)/2), QSize(24,24));
	m_parent = parent;
	m_visible = true;
	m_linked = false;
}

KisListBoxItem::~KisListBoxItem()
{
}

int KisListBoxItem::height(const QListBox *lb) const
{
	return HEIGHT;
}

int KisListBoxItem::width(const QListBox *lb) const
{
	m_size.setWidth(lb -> width() - 1);
	return m_size.width();
}

int KisListBoxItem::height() const
{
	return HEIGHT;
}

int KisListBoxItem::width() const
{
	return m_size.width();
}

void KisListBoxItem::paint(QPainter *gc)
{
	QBrush br = isSelected() ? QBrush::gray : QBrush::lightGray;
	QPoint pt;
	QPixmap *pix;

	gc -> fillRect(0, 0, width() - 1, height() - 1, br);

	m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_visibleRect, m_parent -> colorGroup());
	pt = QPoint(m_visibleRect.left() + 2, m_visibleRect.top() + 2);
	pix = m_visible ? &m_visiblePix : &m_invisiblePix;
	gc -> drawPixmap(pt, *pix, QRect(0, 0, m_visibleRect.width(), m_visibleRect.height()));

	m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_linkedRect, m_parent -> colorGroup());
	pt = QPoint(m_linkedRect.left() + 2, m_linkedRect.top() + 2);
	pix = m_linked ? &m_linkedPix : &m_unlinkedPix;
	gc -> drawPixmap(pt, *pix, QRect(0, 0, m_linkedRect.width(), m_linkedRect.height()));

	m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_previewRect, m_parent -> colorGroup());
	gc -> drawRect(0, 0, width() - 1, height() - 1);
	gc -> drawText(HEIGHT * 3 + 3 * 3, 20, m_name);
}

QPixmap KisListBoxItem::loadPixmap(const QString& filename, const KIconLoader& il)
{
	QPixmap pixmap = il.loadIcon(filename, KIcon::NoGroup);

	if (pixmap.isNull()) {
		QString errmsg = "Can't find " + filename;

		KMessageBox::error(0, i18n(errmsg.latin1()), i18n("Canvas"));
	}

	return pixmap;
}

