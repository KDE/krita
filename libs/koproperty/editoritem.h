/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2004-2006 Jaroslaw Staniek <js@iidea.pl>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPROPERTY_PROPERTYEDITORITEM_H
#define KPROPERTY_PROPERTYEDITORITEM_H

#include "koproperty_global.h"
#include <k3listview.h>

#define KPROPEDITOR_ITEM_MARGIN 2
#define KPROPEDITOR_ITEM_BORDER_COLOR QColor(200,200,200) //! \todo custom color?

template<class U> class Q3AsciiDict;
class QLabel;

namespace KoProperty {

class EditorItemPrivate;
class Property;
class Editor;
class GroupWidget;
class QListViewItem;

/*! \brief Item for a single property displayed within Editor object.
   \author Cedric Pasteur <cedric.pasteur@free.fr>
   \author Alexander Dymo <cloudtemple@mskat.net>
   \author Jaroslaw Staniek <js@iidea.pl>
   @internal
 */
class EditorItem : public K3ListViewItem
{
	public:
		typedef Q3AsciiDict<EditorItem> Dict;

		/*! Creates an EditorItem child of \a parent, associated to \a property.
		 It \a property has not desctiption set, its name (i.e. not i18n'ed) is reused.
		*/
		EditorItem(Editor *editor, EditorItem *parent, Property *property,
			Q3ListViewItem *after=0);

		//! Two helper contructors for subclass
		EditorItem(K3ListView *parent);
		EditorItem(EditorItem *parent, const QString &text);
		EditorItem(EditorItem *parent, EditorItem *after, const QString &text);

		virtual ~EditorItem();

		//! \return a pointer to the property associated to this item.
		Property* property();

	protected:
		/*! Reimplemented from K3ListViewItem to draw custom contents. Properties names are wriiten in bold if
		    modified. Also takes care of drawing borders around the cells as well as pixmaps or colors if necessary.
		*/
		virtual void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);

		/*! Reimplemented from K3ListViewItem to draw custom contents. It takes care of drawing the [+] and [-]
		    signs only if the item has children.
		*/
		virtual void paintBranches(QPainter *p, const QColorGroup &cg, int w, int y, int h);

		virtual void paintFocus(QPainter * p, const QColorGroup & cg, const QRect & r);

		virtual int compare( Q3ListViewItem *i, int col, bool ascending ) const;

		virtual void setHeight( int height );

	protected:
		EditorItemPrivate *d;
};

//! @internal
class EditorGroupItem : public EditorItem
{
	public:
		EditorGroupItem(EditorItem *parent, EditorItem *after, const QString &text, 
			const QString &icon, int sortOrder);
		EditorGroupItem(EditorItem *parent, const QString &text, 
			const QString &icon, int sortOrder);
		virtual ~EditorGroupItem();

//		void  setLabel(QLabel *label) { m_label = label; }
		QWidget*  label() const;

	protected:
		virtual void init(const QString &icon);

		/*! Reimplemented from K3ListViewItem to draw custom contents. */
		virtual void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);
		virtual void setup();
		virtual int compare( Q3ListViewItem *i, int col, bool ascending ) const;

		GroupWidget *m_label;
		int m_sortOrder;
};

//! @internal
class EditorDummyItem : public EditorItem
{
	public:
		EditorDummyItem(K3ListView *parent);
		virtual ~EditorDummyItem();

	protected:
		virtual void setup();
		/*virtual void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);
		virtual void paintFocus(QPainter * p, const QColorGroup & cg, const QRect & r);*/
};

}

#endif
