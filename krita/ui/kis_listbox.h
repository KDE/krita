/*
 *  kis_listbox.h - part of Krita aka Krayon aka KimageShop
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

#if !defined KIS_LISTVIEW_H_
#define KIS_LISTVIEW_H_

#include <qframe.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>

#include <klistbox.h>

class QPainter;
class QWidget;
class KIconLoader;
class KPopupMenu;

class KisListBoxView : public QFrame {
	typedef QFrame super;
	Q_OBJECT

public:
	enum action {VISIBLE, SELECTION, LINKING, PROPERTIES, ADD, REMOVE, ADDMASK, REMOVEMASK, UPPER, LOWER, FRONT, BACK, LEVEL};
	enum flags {SHOWVISIBLE = 1, SHOWLINKED = (1 << 1), SHOWPREVIEW = (1 << 2), SHOWMASK = (1 << 3), SHOWALL = (SHOWMASK|SHOWPREVIEW|SHOWLINKED|SHOWVISIBLE)};

	KisListBoxView(const QString& label, flags f = SHOWALL, QWidget *parent = 0, const char *name = 0);
	virtual ~KisListBoxView();

private slots:
	void slotMenuAction(int mnuId);
	void slotAboutToShow();
	void slotShowContextMenu(QListBoxItem *item, const QPoint& pos);
	void slotExecuted(QListBoxItem *item, const QPoint& pos);

private:
	KListBox *m_lst;
	KPopupMenu *m_contextMnu;
};

class KisListBoxItem : public QListBoxItem {
	typedef QListBoxItem super;

public:
	KisListBoxItem(const QString& label, QListBox *parent, int flags = 0);
	virtual ~KisListBoxItem();

	virtual int height(const QListBox *lb) const;
	virtual int width(const QListBox *lb) const;
	virtual int height() const;
	virtual int width() const;
	virtual void paint(QPainter *gc);

	bool intersectVisibleRect(const QPoint& pos, int yOffset) const;
	bool intersectLinkedRect(const QPoint& pos, int yOffset) const;
	bool intersectPreviewRect(const QPoint& pos, int yOffset) const;

private:
	void init(const QString& label, QListBox *parent, int flags);
	QPixmap loadPixmap(const QString& filename, const KIconLoader& il);
	bool intersectRect(const QRect& rc, const QPoint& pos, int yOffset) const;

private:
	mutable QSize m_size;
	QString m_label;
	QPixmap m_visiblePix;
	QPixmap m_invisiblePix;
	QPixmap m_linkedPix;
	QPixmap m_unlinkedPix;
	QPixmap m_preview;
	QRect m_visibleRect;
	QRect m_linkedRect;
	QRect m_previewRect;
	QWidget *m_parent;
	bool m_visible;
	bool m_linked;
};

#endif // __kis_layerview_h__

