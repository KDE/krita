/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KPROPERTY_PROPERTYEDITOR_H
#define KPROPERTY_PROPERTYEDITOR_H

#include <qpointer.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QByteArray>
#include "koproperty_global.h"

#ifdef QT_ONLY
#include <q3listview.h>
#else
#include <k3listview.h>
//#define QListView K3ListView
#endif

class QSize;

namespace KoProperty {

class EditorPrivate;
class Property;
class Set;
class Widget;
class EditorItem;

//! \brief A listview to edit properties
/*! Editor uses property options using Property::option(const char *)
    to override default behaviour of editor items.
    Currently supported options are:
    <ul><li> min: integer setting for minimum value of IntEdit and DoubleEdit item. Default is 0.
    Set "min" to -1, if you want this special value to be allowed.</li>
    <li> minValueText: i18n'd QString used in IntEdit to set "specialValueText"
            widget's property</li>
    <li> max: integer setting for minimum value of IntEdit item. Default is 0xffff.</li>
    <li> precision:  The number of decimals after the decimal point. (for DoubleEdit)</li>
    <li> step : the size of the step that is taken when the user hits the up
    or down buttons (for DoubleEdit) </li></ul>
   \author Cedric Pasteur <cedric.pasteur@free.fr>
   \author Alexander Dymo <cloudtemple@mskat.net>
   \author Jaroslaw Staniek <js@iidea.pl>
 */
class KOPROPERTY_EXPORT Editor : public K3ListView
{
	Q_OBJECT

	public:
		/*! Creates an empty Editor with \a parent as parent widget.
		If \a autoSync == true, properties values are automatically synced as
		soon as editor contents change (eg the user types text, etc.)
		and the values are written in the property set. Otherwise, property set
		is updated only when selected item changes or user presses Enter key.
		Each property can overwrite this if its autoSync() == 0 or 1.
		*/
		Editor(QWidget *parent=0, bool autoSync=true, const char *name=0);

		virtual ~Editor();

		virtual QSize sizeHint() const;
		virtual void setFocus();

	public slots:
		/*! Populates the editor with an item for each property in the List.
		  Also creates child items for composed properties.
		*/
		void changeSet(Set *set, bool preservePrevSelection=false);

		/*! Clears all items in the list.
		   if \a editorOnly is true, then only the current editor will be cleared,
			not the whole list.
		*/
		void clear(bool editorOnly = false);

		/*! Accept the changes mae to the current editor (as if the user had pressed Enter key) */
		void acceptInput();

	signals:
		/*! Emitted when current property set has been changed. May be 0. */
		void propertySetChanged(KoProperty::Set *set);

	protected slots:
		/*! Updates property widget in the editor.*/
		void slotPropertyChanged(KoProperty::Set& set, KoProperty::Property& property);

		void slotPropertyReset(KoProperty::Set& set, KoProperty::Property& property);

		/*! Updates property in the list when new value is selected in the editor.*/
		void slotWidgetValueChanged(Widget* widget);

		/*! Called when the user presses Enter to accet the input
		   (only applies when autoSync() == false).*/
		void slotWidgetAcceptInput(Widget *widget);

		/*! Called when the user presses Esc. Calls undo(). */
		void slotWidgetRejectInput(Widget *widget);

		/*! Called when current property set is about to be cleared. */
		void slotSetWillBeCleared();

		/*! Called when current property set is about to be destroyed. */
		void slotSetWillBeDeleted();

		/*! This slot is called when the user clicks the list view.
		   It takes care of deleting current editor and
		   creating a new editor for the newly selected item. */
		void slotClicked(Q3ListViewItem *item);

		/*! Undoes the last change in property editor.*/
		void undo();

		void updateEditorGeometry(bool forceUndoButtonSettings = false, bool undoButtonVisible = false);
		void updateEditorGeometry(EditorItem *item, Widget* widget, bool forceUndoButtonSettings = false, bool undoButtonVisible = false);

		void hideEditor();

		void slotCollapsed(Q3ListViewItem *item);
		void slotExpanded(Q3ListViewItem *item);
		void slotColumnSizeChanged(int section);
		void slotColumnSizeChanged(int section, int oldSize, int newSize);
		void slotCurrentChanged(Q3ListViewItem *item);
		void changeSetLater();
		void selectItemLater();
	protected:
		/*! \return \ref Widget for given property.
		Uses cache to store created widgets.
		Cache will be cleared only with clearWidgetCache().*/
		Widget *createWidgetForProperty(Property *property, bool changeWidgetProperty=true);

		/*! Deletes cached machines.*/
		void clearWidgetCache();

		void fill();
		void addItem(const QByteArray &name, EditorItem *parent);

		void showUndoButton( bool show );

		virtual void resizeEvent(QResizeEvent *ev);
		virtual bool eventFilter( QObject * watched, QEvent * e );
		bool handleKeyPress(QKeyEvent* ev);

		virtual bool event( QEvent * e );
		void updateFont();

		virtual void contentsMousePressEvent( QMouseEvent * e );

	private:
		EditorPrivate *d;

	friend class EditorItem;
	friend class Widget;
};

}

#endif
