/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KPROPERTY_PIXMAPEDIT_H
#define KPROPERTY_PIXMAPEDIT_H

#include "../widget.h"
#include <QPixmap>
#include <QVariant>
//Added by qt3to4:
#include <QLabel>
#include <QResizeEvent>
#include <QEvent>

class QLabel;
class QPushButton;

namespace KoProperty {

class KOPROPERTY_EXPORT PixmapEdit : public Widget
{
	Q_OBJECT

	public:
		PixmapEdit(Property *property, QWidget *parent=0);
		virtual ~PixmapEdit();

		virtual QVariant value() const;
		virtual void setValue(const QVariant &value, bool emitChange=true);
		virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

		void resizeEvent(QResizeEvent *ev);
		bool eventFilter(QObject *o, QEvent *ev);

	protected:
		virtual void setReadOnlyInternal(bool readOnly);

	protected slots:
		/*! Helper used by selectPixmap(). Can be also used by subclassess. 
		 Selected path will be stored in "lastVisitedImagePath" config entry within "Recent Dirs" 
		 config group of application's settings. This entry can be later reused when file dialogs 
		 are opened for selecting image files. */
		QString selectPixmapFileName();

		/*! Selects a new pixmap using "open" file dialog. Can be reimplemented. */
		virtual void selectPixmap();

	protected:
		QLabel *m_edit;
		QLabel *m_popup;
		QPushButton *m_button;
		QVariant m_recentlyPainted;
		QPixmap m_pixmap, m_scaledPixmap, m_previewPixmap;
};

}

#endif
