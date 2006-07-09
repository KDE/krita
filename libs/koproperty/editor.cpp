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

#include "editor.h"
#include "editoritem.h"
#include "set.h"
#include "factory.h"
#include "property.h"
#include "widget.h"

#include <QPushButton>
#include <QLayout>
#include <QMap>
#include <QPointer>
#include <q3header.h>
#include <q3asciidict.h>
#include <QToolTip>
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QLabel>
#include <QByteArray>
#include <QEvent>
#include <QKeyEvent>
#include <Q3ValueList>
#include <QResizeEvent>
#include <QMouseEvent>

#ifdef QT_ONLY
#else
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kapplication.h>
#endif

namespace KoProperty {

//! @internal
static bool kofficeAppDirAdded = false;

//! \return true if \a o has parent \a par.
//! @internal
inline bool hasParent(QObject* par, QObject* o)
{
	if (!o || !par)
		return false;
	while (o && o != par)
		o = o->parent();
	return o == par;
}

class EditorPrivate
{
	public:
		EditorPrivate(Editor *editor)
		: itemDict(101, false), justClickedItem(false)
		{
			currentItem = 0;
			undoButton = 0;
			topItem = 0;
			itemToSelectLater = 0;
			if (!kofficeAppDirAdded) {
				kofficeAppDirAdded = true;
				KGlobal::iconLoader()->addAppDir("koffice");
			}
			previouslyCollapsedGroupItem = 0;
			childFormPreviouslyCollapsedGroupItem = 0;
			slotPropertyChanged_enabled = true;
			QObject::connect(&changeSetLaterTimer, SIGNAL(timeout()),
				editor, SLOT(changeSetLater()));
		}
		~EditorPrivate()
		{
		}

		QPointer<Set> set;
		//! widget cache for property types, widget will be deleted
		QMap<Property*, Widget* >  widgetCache;
		QPointer<Widget> currentWidget;
		EditorItem *currentItem;
		EditorItem *topItem; //! The top item is used to control the drawing of every branches.
		QPushButton *undoButton; //! "Revert to defaults" button
		EditorItem::Dict itemDict;

		int baseRowHeight;
		bool sync : 1;
		bool insideSlotValueChanged : 1;

		//! Helpers for changeSetLater()
		QTimer changeSetLaterTimer;
		bool setListLater_set : 1;
		bool preservePrevSelection_preservePrevSelection : 1;
		QByteArray preservePrevSelection_propertyToSelect;
		//bool doNotSetFocusOnSelection : 1;
		//! Used in setFocus() to prevent scrolling to previously selected item on mouse click
		bool justClickedItem : 1;
		//! Helper for slotWidgetValueChanged()
		bool slotPropertyChanged_enabled : 1;
		//! Helper for changeSet()
		Set* setListLater_list;
		//! used by selectItemLater()
		EditorItem *itemToSelectLater;

		Q3ListViewItem *previouslyCollapsedGroupItem;
		Q3ListViewItem *childFormPreviouslyCollapsedGroupItem;
};
}

using namespace KoProperty;

Editor::Editor(QWidget *parent, bool autoSync, const char *name)
 : K3ListView(parent)
{
	setObjectName(name);
	d = new EditorPrivate(this);
	d->itemDict.setAutoDelete(false);

	d->set = 0;
	d->topItem = 0;
	d->currentItem = 0;
	d->sync = autoSync;
	d->insideSlotValueChanged = false;
	d->setListLater_set = false;
	d->preservePrevSelection_preservePrevSelection = false;
	d->setListLater_list = 0;

	d->undoButton = new QPushButton(viewport());
	d->undoButton->setFocusPolicy(Qt::NoFocus);
	setFocusPolicy(Qt::ClickFocus);
	d->undoButton->setMinimumSize(QSize(5,5)); // allow to resize undoButton even below pixmap size
	d->undoButton->setIcon(SmallIcon("undo"));
	d->undoButton->setToolTip( i18n("Undo changes"));
	d->undoButton->hide();
	connect(d->undoButton, SIGNAL(clicked()), this, SLOT(undo()));

	installEventFilter(this);
	viewport()->installEventFilter(this);

	addColumn(i18n("Name"));
	addColumn(i18n("Value"));
	setAllColumnsShowFocus(true);
	setColumnWidthMode(0, Q3ListView::Maximum);
	setFullWidth(true);
	setShowSortIndicator(false);
	setShadeSortColumn(false);
	setTooltipColumn(0);
	setSorting(0);
	setItemMargin(KPROPEDITOR_ITEM_MARGIN);
	header()->setMovingEnabled( false );
	setTreeStepSize(16 + 2/*left*/ + 1/*right*/);

	updateFont();
//	d->baseRowHeight = QFontMetrics(font()).height() + itemMargin()*2;

	connect(this, SIGNAL(selectionChanged(Q3ListViewItem *)), this, SLOT(slotClicked(Q3ListViewItem *)));
	connect(this, SIGNAL(currentChanged(Q3ListViewItem *)), this, SLOT(slotCurrentChanged(Q3ListViewItem *)));
	connect(this, SIGNAL(expanded(Q3ListViewItem *)), this, SLOT(slotExpanded(Q3ListViewItem *)));
	connect(this, SIGNAL(collapsed(Q3ListViewItem *)), this, SLOT(slotCollapsed(Q3ListViewItem *)));
	connect(header(), SIGNAL(sizeChange(int, int, int)), this, SLOT(slotColumnSizeChanged(int, int, int)));
//	connect(header(), SIGNAL(clicked(int)), this, SLOT(updateEditorGeometry()));
//	connect(header(), SIGNAL(clicked(int)), this, SLOT(updateEditorGeometryAndGroupLabels()));
	connect(header(), SIGNAL(sectionHandleDoubleClicked (int)), this, SLOT(slotColumnSizeChanged(int)));
}

Editor::~Editor()
{
	clearWidgetCache();
	delete d;
	d = 0;
}

void
Editor::fill()
{
	setUpdatesEnabled(false);
	d->itemToSelectLater = 0;
	qApp->processEvents(QEventLoop::AllEvents);
	hideEditor();
	K3ListView::clear();
	d->itemDict.clear();
	clearWidgetCache();
	if(!d->set) {
		d->topItem = 0;
		setUpdatesEnabled(true);
		triggerUpdate();
		return;
	}

	d->topItem = new EditorDummyItem(this);

	const Q3ValueList<QByteArray> groupNames = d->set->groupNames();
//	kopropertydbg << "Editor::fill(): group names = " << groupNames.count() << endl;
	if(groupNames.count() == 1) { // one group (default one), so don't show groups
		//add flat set of properties
		const Q3ValueList<QByteArray>& propertyNames = d->set->propertyNamesForGroup( groupNames.first() );
		Q3ValueListConstIterator<QByteArray> it = propertyNames.constBegin();
		for( ; it != propertyNames.constEnd(); ++it)
			addItem(*it, d->topItem);
	}
	else { // create a groupItem for each group
		EditorGroupItem *prevGroupItem = 0;
		int sortOrder = 0;
		for (Q3ValueListConstIterator<QByteArray> it = groupNames.constBegin(); it!=groupNames.constEnd(); 
			++it, sortOrder++) 
		{
			const Q3ValueList<QByteArray>& propertyNames = d->set->propertyNamesForGroup(*it);
			EditorGroupItem *groupItem;
			if (prevGroupItem)
				groupItem = new EditorGroupItem(d->topItem, prevGroupItem, 
					d->set->groupDescription(*it), d->set->groupIcon(*it), sortOrder );
			else
				groupItem = new EditorGroupItem(d->topItem, 
					d->set->groupDescription(*it), d->set->groupIcon(*it), sortOrder );

			Q3ValueList<QByteArray>::ConstIterator it2 = propertyNames.constBegin();
			for( ; it2 != propertyNames.constEnd(); ++it2)
				addItem(*it2, groupItem);

			prevGroupItem = groupItem;
		}
	}

//	repaint();

	if (firstChild())
	{
		setCurrentItem(firstChild());
		setSelected(firstChild(), true);
		slotClicked(firstChild());
		updateGroupLabelsPosition();
	}
	setUpdatesEnabled(true);
	// aaah, call this instead of update() as explained here http://lists.trolltech.com/qt-interest/2000-06/thread00337-0.html
	triggerUpdate();
}

void
Editor::addItem(const QByteArray &name, EditorItem *parent)
{
	if(!d->set || !d->set->contains(name))
		return;

	Property *property = &(d->set->property(name));
	if(!property || !property->isVisible()) {
//		kopropertydbg << "Property is not visible: " << name << endl;
		return;
	}
	Q3ListViewItem *last = parent ? parent->firstChild() : d->topItem->firstChild();
	while(last && last->nextSibling())
		last = last->nextSibling();

	EditorItem *item=0;
	if(parent)
		item = new EditorItem(this, parent, property, last);
	else
		item = new EditorItem(this, d->topItem, property, last);
	d->itemDict.insert(name, item);

	// Create child items
	item->setOpen(true);
	if(!property->children())
		return;

	last = 0;
	Q3ValueList<Property*>::ConstIterator endIt = property->children()->constEnd();
	for(Q3ValueList<Property*>::ConstIterator it = property->children()->constBegin(); it != endIt; ++it) {
		//! \todo allow to have child prop with child items too
		if( *it && (*it)->isVisible() )
			last = new EditorItem(this, item, *it, last);
	}
}

void
Editor::changeSet(Set *set, bool preservePrevSelection)
{
	changeSetInternal(set, preservePrevSelection, "");
}

void
Editor::changeSet(Set *set, const QByteArray& propertyToSelect)
{
	changeSetInternal(set, !propertyToSelect.isEmpty(), propertyToSelect);
}

void
Editor::changeSetInternal(Set *set, bool preservePrevSelection, const QByteArray& propertyToSelect)
{
	if (d->insideSlotValueChanged) {
		//changeSet() called from inside of slotValueChanged()
		//this is dangerous, because there can be pending events,
		//especially for the GUI stuff, so let's do delayed work
		d->setListLater_list = set;
		d->preservePrevSelection_preservePrevSelection = preservePrevSelection;
		d->preservePrevSelection_propertyToSelect = propertyToSelect;
		qApp->processEvents(QEventLoop::AllEvents);
		if (d->set) {
			//store prev. selection for this prop set
			if (d->currentItem)
				d->set->setPrevSelection( d->currentItem->property()->name() );
			kDebug() << d->set->prevSelection() << endl;
		}
		if (!d->setListLater_set) {
			d->setListLater_set = true;
			d->changeSetLaterTimer.setSingleShot(true);
			d->changeSetLaterTimer.start(10);
		}
		return;
	}

	if (d->set) {
		slotWidgetAcceptInput(d->currentWidget);
		//store prev. selection for this prop set
		if (d->currentItem)
			d->set->setPrevSelection( d->currentItem->property()->name() );
		else
			d->set->setPrevSelection( "" );
		d->set->disconnect(this);
	}

	QByteArray selectedPropertyName1 = propertyToSelect, selectedPropertyName2 = propertyToSelect;
	if (preservePrevSelection) {
		//try to find prev. selection:
		//1. in new list's prev. selection
		if(set)
			selectedPropertyName1 = set->prevSelection();
		//2. in prev. list's current selection
		if(d->set)
			selectedPropertyName2 = d->set->prevSelection();
	}

	d->set = set;
	if (d->set) {
		//receive property changes
		connect(d->set, SIGNAL(propertyChangedInternal(KoProperty::Set&, KoProperty::Property&)),
			this, SLOT(slotPropertyChanged(KoProperty::Set&, KoProperty::Property&)));
		connect(d->set, SIGNAL(propertyReset(KoProperty::Set&, KoProperty::Property&)),
			this, SLOT(slotPropertyReset(KoProperty::Set&, KoProperty::Property&)));
		connect(d->set,SIGNAL(aboutToBeCleared()), this, SLOT(slotSetWillBeCleared()));
		connect(d->set,SIGNAL(aboutToBeDeleted()), this, SLOT(slotSetWillBeDeleted()));
	}

	fill();

	emit propertySetChanged(d->set);

	if (d->set) {
		//select prev. selected item
		EditorItem * item = 0;
		if (!selectedPropertyName2.isEmpty()) //try other one for old prop set
			item = d->itemDict[selectedPropertyName2];
		if (!item && !selectedPropertyName1.isEmpty()) //try old one for current prop set
			item = d->itemDict[selectedPropertyName1];

		if (item) {
			d->itemToSelectLater = item;
			QTimer::singleShot(10, this, SLOT(selectItemLater()));
			//d->doNotSetFocusOnSelection = !hasParent(this, focusWidget());
			//setSelected(item, true);
			//d->doNotSetFocusOnSelection = false;
//			ensureItemVisible(item);
		}
	}
}

//! @internal
void Editor::selectItemLater()
{
	if (!d->itemToSelectLater)
		return;
	EditorItem *item = d->itemToSelectLater;
	d->itemToSelectLater = 0;
	setSelected(item, true);
	ensureItemVisible(item);
}

//! @internal
void
Editor::changeSetLater()
{
	qApp->processEvents(QEventLoop::AllEvents);
	if (kapp->hasPendingEvents()) {
		d->changeSetLaterTimer.setSingleShot(true);
		d->changeSetLaterTimer.start(10); //try again...
		return;
	}
	d->setListLater_set = false;
	if (!d->setListLater_list)
		return;

	bool b = d->insideSlotValueChanged;
	d->insideSlotValueChanged = false;
	changeSet(d->setListLater_list, d->preservePrevSelection_preservePrevSelection);
	changeSetInternal(d->setListLater_list, d->preservePrevSelection_preservePrevSelection, 
		d->preservePrevSelection_propertyToSelect);
	d->insideSlotValueChanged = b;
}

void
Editor::clear(bool editorOnly)
{
	d->itemToSelectLater = 0;
	hideEditor();

	if(!editorOnly) {
		qApp->processEvents(QEventLoop::AllEvents);
		if(d->set)
			d->set->disconnect(this);
		clearWidgetCache();
		K3ListView::clear();
		d->itemDict.clear();
		d->topItem = 0;
	}
}

void
Editor::undo()
{
	if(!d->currentWidget || !d->currentItem || (d->set && d->set->isReadOnly()) || (d->currentWidget && d->currentWidget->isReadOnly()))
		return;

	int propertySync = d->currentWidget->property()->autoSync();
	bool sync = (propertySync != 0 && propertySync != 1) ?
				 d->sync : (propertySync!=0);

	if(sync)
		d->currentItem->property()->resetValue();
	if (d->currentWidget && d->currentItem) {//(check because current widget could be removed by resetValue())
		d->currentWidget->setValue( d->currentItem->property()->value());
		repaintItem(d->currentItem);
	}
}

void
Editor::slotPropertyChanged(Set& set, Property& property)
{
	if (!d->slotPropertyChanged_enabled)
		return;
	if(&set != d->set)
		return;

	if (d->currentItem && d->currentItem->property() == &property) {
		d->currentWidget->setValue(property.value(), false);
		for(Q3ListViewItem *item = d->currentItem->firstChild(); item; item = item->nextSibling())
			repaintItem(item);
	}
	else  {
		// prop not in the dict, might be a child property:
		EditorItem *item = d->itemDict[property.name()];
		if(!item && property.parent())
			item = d->itemDict[property.parent()->name()];
		if (item) {
			repaintItem(item);
			for(Q3ListViewItem *it = item->firstChild(); it; it = it->nextSibling())
				repaintItem(it);
		}
	}

//! @todo should we move this somewhere?
#if 0
	if (property.parent() && property.parent()->type()==Rect) {
		const int delta = property.value().toInt()-previousValue.toInt();
		if (property.type()==Rect_X) { //|| property.type()==Rect_Y)
			property.parent()->child("width")->setValue(delta, false);
		}

/*	if (widget->property() && (QWidget*)d->currentWidget==widget && d->currentItem->parent()) {
		EditorItem *parentItem = static_cast<EditorItem*>(d->currentItem->parent());
		const int thisType = ;
			&& parentItem->property()->type()==Rect) {
			//changing x or y components of Rect type shouldn't change width or height, respectively
			if (thisType==Rect_X) {
				EditorItem *rectWidthItem = static_cast<EditorItem*>(d->currentItem->nextSibling()->nextSibling());
				if (delta!=0) {
					rectWidthItem->property()->setValue(rectWidthItem->property()->value().toInt()+delta, false);
				}
			}
		}*/
	}
#endif
	showUndoButton( property.isModified() );
}

void
Editor::slotPropertyReset(Set& set, Property& property)
{
	if(&set != d->set)
		return;

	if (d->currentItem && d->currentItem->property() == &property) {
		d->currentWidget->setValue(property.value(), false);
		for(Q3ListViewItem *item = d->currentItem->firstChild(); item; item = item->nextSibling())
			repaintItem(item);
	}
	else  {
		EditorItem *item = d->itemDict[property.name()];
		// prop not in the dict, might be a child prop.
		if(!item && property.parent())
			item = d->itemDict[property.parent()->name()];
		if (item) {
			repaintItem(item);
			for(Q3ListViewItem *it = item->firstChild(); it; it = it->nextSibling())
				repaintItem(it);
		}
	}

	showUndoButton( false );
}

void
Editor::slotWidgetValueChanged(Widget *widget)
{
	if(!widget || !d->set || (d->set && d->set->isReadOnly()) || (widget && widget->isReadOnly()) || !widget->property())
		return;

	d->insideSlotValueChanged = true;

	QVariant value = widget->value();
	int propertySync = widget->property()->autoSync();
	bool sync = (propertySync != 0 && propertySync != 1) ?
				 d->sync : (propertySync!=0);

	if(sync) {
		d->slotPropertyChanged_enabled = false;
		QPointer<Widget> pWidget = widget; //safe, widget can be destroyed in the meantime
		widget->property()->setValue(value);
		if (pWidget)
		  showUndoButton( pWidget->property()->isModified() );
		d->slotPropertyChanged_enabled = true;
	}

	d->insideSlotValueChanged = false;
}

void
Editor::acceptInput()
{
	slotWidgetAcceptInput(d->currentWidget);
}

void
Editor::slotWidgetAcceptInput(Widget *widget)
{
	if(!widget || !d->set || !widget->property() || (d->set && d->set->isReadOnly()) || (widget && widget->isReadOnly()))
		return;

	widget->property()->setValue(widget->value());
}

void
Editor::slotWidgetRejectInput(Widget *widget)
{
	if(!widget || !d->set)
		return;

	undo();
}

void
Editor::slotClicked(Q3ListViewItem *it)
{
	d->previouslyCollapsedGroupItem = 0;
	d->childFormPreviouslyCollapsedGroupItem = 0;

	acceptInput();

	hideEditor();
	if(!it)
		return;

	EditorItem *item = static_cast<EditorItem*>(it);
	Property *p = item->property();
	if(!p)
		return;

	d->currentItem = item;
	d->currentWidget = createWidgetForProperty(p);

	//moved up updateEditorGeometry();
	showUndoButton( p->isModified() );
	if (d->currentWidget) {
		if (d->currentWidget->visibleFlag()) {
			d->currentWidget->show();
			if (hasParent( this, kapp->focusWidget() ))
				d->currentWidget->setFocus();
		}
	}

	d->justClickedItem = true;
}

void
Editor::slotCurrentChanged(Q3ListViewItem *item)
{
	if (item == firstChild()) {
		Q3ListViewItem *oldItem = item;
		while (item && (!item->isSelectable() || !item->isVisible()))
			item = item->itemBelow();
		if (item && item != oldItem) {
			setSelected(item,true);
			return;
		}
	}
}

void
Editor::slotSetWillBeCleared()
{
	d->itemToSelectLater = 0;
	if (d->currentWidget) {
		acceptInput();
		d->currentWidget->setProperty(0);
	}
	clear();
}

void
Editor::slotSetWillBeDeleted()
{
	clear();
	d->set = 0;
}

Widget*
Editor::createWidgetForProperty(Property *property, bool changeWidgetProperty)
{
//	int type = property->type();
	QPointer<Widget> widget = d->widgetCache[property];

	if(!widget) {
		widget = FactoryManager::self()->createWidgetForProperty(property);
		if (!widget)
			return 0;
		widget->setReadOnly( (d->set && d->set->isReadOnly()) || property->isReadOnly() );
		d->widgetCache[property] = widget;
		widget->setProperty(0); // to force reloading property later
		widget->hide();
		connect(widget, SIGNAL(valueChanged(Widget*)),
			this, SLOT(slotWidgetValueChanged(Widget*)) );
		connect(widget, SIGNAL(acceptInput(Widget*)),
			this, SLOT(slotWidgetAcceptInput(Widget*)) );
		connect(widget, SIGNAL(rejectInput(Widget*)),
			this, SLOT(slotWidgetRejectInput(Widget*)) );
	}

	//update geometry earlier, because Widget::setValue() can depend on widget's geometry
	updateEditorGeometry(d->currentItem, widget);

	if(widget && (!widget->property() || changeWidgetProperty))
		widget->setProperty(property);

//	if (!d->doNotSetFocusOnSelection) {
//		widget->setFocus();
//	}

	return widget;
}


void
Editor::clearWidgetCache()
{
	for(QMap<Property*, Widget*>::iterator it = d->widgetCache.begin(); it != d->widgetCache.end(); ++it)
		it.value()->deleteLater();
//		delete it.data();
	d->widgetCache.clear();
}

void
Editor::updateEditorGeometry(bool forceUndoButtonSettings, bool undoButtonVisible)
{
	updateEditorGeometry(d->currentItem, d->currentWidget,
		forceUndoButtonSettings, undoButtonVisible);
}

void
Editor::updateEditorGeometry(EditorItem *item, Widget* widget,
  bool forceUndoButtonSettings, bool undoButtonVisible)
{
	if(!item || !widget)
		return;

	int placeForUndoButton;
	if (forceUndoButtonSettings ? undoButtonVisible : d->undoButton->isVisible())
		placeForUndoButton = d->undoButton->width();
	else
		placeForUndoButton = widget->leavesTheSpaceForRevertButton() ? d->undoButton->width() : 0;

	QRect r;
	int y = itemPos(item);
	r.setX(header()->sectionPos(1)-(widget->hasBorders()?1:0)); //-1, to align to horizontal line
	r.setY(y-(widget->hasBorders()?1:0));
	r.setWidth(header()->sectionSize(1)+(widget->hasBorders()?1:0) //+1 because we subtracted 1 from X
		- placeForUndoButton);
	r.setHeight(item->height()+(widget->hasBorders()?1:-1));

	// check if the column is fully visible
	if (visibleWidth() < r.right())
		r.setRight(visibleWidth());

	moveChild(widget, r.x(), r.y());
	widget->resize(r.size());
	qApp->processEvents(QEventLoop::AllEvents);
}

void
Editor::updateGroupLabelsPosition()
{
	if(!d->topItem || d->itemDict.isEmpty())
		return;

	EditorGroupItem *group = dynamic_cast<EditorGroupItem*>(d->topItem->firstChild());
	while(group) {
		QRect r = itemRect((Q3ListViewItem*) group);
		if(group->label()) {
			group->label()->setGeometry(r);
			group->label()->repaint();
		}
		group = dynamic_cast<EditorGroupItem*>(group->nextSibling());
	}
}

void
Editor::hideEditor()
{
	d->currentItem = 0;
	QWidget *cw = d->currentWidget;
	if(cw) {
		d->currentWidget = 0;
		cw->hide();
	}
	d->undoButton->hide();
}

void
Editor::showUndoButton( bool show )
{
	if (!d->currentItem || !d->currentWidget || (d->currentWidget && d->currentWidget->isReadOnly()))
		return;

	int y = viewportToContents(QPoint(0, itemRect(d->currentItem).y())).y();
	QRect geometry(columnWidth(0), y, columnWidth(1) + 1, d->currentItem->height());
	d->undoButton->resize(d->baseRowHeight, d->baseRowHeight);

	updateEditorGeometry(true, show);

	if (!show) {
/*	  if (d->currentWidget) {
			if (d->currentWidget->leavesTheSpaceForRevertButton()) {
				geometry.setWidth(geometry.width()-d->undoButton->width());
			}
			d->currentWidget->resize(geometry.width(), geometry.height());
		}*/
		d->undoButton->hide();
		return;
	}

	QPoint p = contentsToViewport(QPoint(0, geometry.y()));
	d->undoButton->move(geometry.x() + geometry.width()
		-((d->currentWidget && d->currentWidget->hasBorders())?1:0)/*editor is moved by 1 to left*/
		- d->undoButton->width(), p.y());
//  if (d->currentWidget) {
//	  d->currentWidget->move(d->currentWidget->x(), p.y());
//	  d->currentWidget->resize(geometry.width()-d->undoButton->width(), geometry.height());
//  }
	d->undoButton->show();
}

void
Editor::slotExpanded(Q3ListViewItem *item)
{
	if (!item)
		return;

	//select child item again if a group item has been expanded
	if (!selectedItem() && dynamic_cast<EditorGroupItem*>(item) && d->previouslyCollapsedGroupItem == item
		&& d->childFormPreviouslyCollapsedGroupItem) {
			setSelected(d->childFormPreviouslyCollapsedGroupItem, true);
			setCurrentItem(selectedItem());
			slotClicked(selectedItem());
	}
	updateEditorGeometry();
	updateGroupLabelsPosition();
	repaintContents();
	repaint();
}

void
Editor::slotCollapsed(Q3ListViewItem *item)
{
	if (!item)
		return;
	//unselect child item and hide editor if a group item has been collapsed
	if (dynamic_cast<EditorGroupItem*>(item)) {
		for (Q3ListViewItem *i = selectedItem(); i; i = i->parent()) {
			if (i->parent()==item) {
				d->previouslyCollapsedGroupItem = item;
				d->childFormPreviouslyCollapsedGroupItem = selectedItem();
				hideEditor();
				setSelected(selectedItem(), false);
				setSelected(item->nextSibling(), true);
				break;
			}
		}
	}
	updateEditorGeometry();
	updateGroupLabelsPosition();
	repaintContents();
	repaint();
}

void
Editor::slotColumnSizeChanged(int section, int oldSize, int newSize)
{
	Q_UNUSED(section);
	Q_UNUSED(oldSize);
	Q_UNUSED(newSize);
	/*for (Q3ListViewItemIterator it(this); it.current(); ++it) {
//		if (section == 0 && dynamic_cast<EditorGroupItem*>(it.current())) {
//			it.current()->repaint();
//	}
	}*/
/*
	if(d->currentWidget) {
		if(section == 0)
			d->currentWidget->move(newS, d->currentWidget->y());
		else  {
			if(d->undoButton->isVisible())
				d->currentWidget->resize(newS - d->undoButton->width(), d->currentWidget->height());
			else
				d->currentWidget->resize(
					newS-(d->currentWidget->leavesTheSpaceForRevertButton() ? d->undoButton->width() : 0),
					d->currentWidget->height());
		}
	}*/
//	repaintContents();
//	repaint();
	updateEditorGeometry();
	update();
}

void
Editor::slotColumnSizeChanged(int section)
{
	setColumnWidth(1, viewport()->width() - columnWidth(0));
	slotColumnSizeChanged(section, 0, header()->sectionSize(section));

/*  if(d->currentWidget) {
		if(d->undoButton->isVisible())
			d->currentWidget->resize(columnWidth(1) - d->undoButton->width(), d->currentWidget->height());
		else
			d->currentWidget->resize(
				columnWidth(1)-(d->currentWidget->leavesTheSpaceForRevertButton() ? d->undoButton->width() : 0),
				d->currentWidget->height());
	}*/
	if(d->undoButton->isVisible())
		showUndoButton(true);
	else
		updateEditorGeometry();
}

QSize
Editor::sizeHint() const
{
	return QSize( QFontMetrics(font()).width(columnText(0)+columnText(1)+"   "),
		K3ListView::sizeHint().height());
}

void
Editor::setFocus()
{
	EditorItem *item = static_cast<EditorItem *>(selectedItem());
	if (item) {
		if (!d->justClickedItem)
			ensureItemVisible(item);
		d->justClickedItem = false;
	}
	else {
		//select an item before focusing
		item = static_cast<EditorItem *>(itemAt(QPoint(10,1)));
		if (item) {
			ensureItemVisible(item);
			setSelected(item, true);
		}
	}
	if (d->currentWidget) {
//		kopropertydbg << "d->currentWidget->setFocus()" << endl;
		d->currentWidget->setFocus();
	}
	else {
//		kopropertydbg << "K3ListView::setFocus()" << endl;
		K3ListView::setFocus();
	}
}

void
Editor::resizeEvent(QResizeEvent *ev)
{
	K3ListView::resizeEvent(ev);
	if(d->undoButton->isVisible())
		showUndoButton(true);
	update();
	updateGroupLabelsPosition();
}

bool
Editor::eventFilter( QObject * watched, QEvent * e )
{
	if ((watched==this || watched==viewport()) && e->type()==QEvent::KeyPress) {
		if (handleKeyPress(static_cast<QKeyEvent*>(e)))
			return true;
	}
	return K3ListView::eventFilter(watched, e);
}

bool
Editor::handleKeyPress(QKeyEvent* ev)
{
	const int k = ev->key();
	const Qt::KeyboardModifiers s = ev->modifiers();

	//selection moving
	Q3ListViewItem *item = 0;

	if ( ((s == Qt::NoModifier) && (k == Qt::Key_Up)) || (k==Qt::Key_Backtab) ) {
		//find prev visible
		item = selectedItem() ? selectedItem()->itemAbove() : 0;
		while (item && (!item->isSelectable() || !item->isVisible()))
			item = item->itemAbove();
		if (!item)
			return true;
	}
	else if( (s == Qt::NoModifier) && ((k == Qt::Key_Down) || (k == Qt::Key_Tab)) ) {
		//find next visible
		item = selectedItem() ? selectedItem()->itemBelow() : 0;
		while (item && (!item->isSelectable() || !item->isVisible()))
			item = item->itemBelow();
		if (!item)
			return true;
	}
	else if( (s == Qt::NoModifier) && (k==Qt::Key_Home) ) {
		if (d->currentWidget && d->currentWidget->hasFocus())
			return false;
		//find 1st visible
		item = firstChild();
		while (item && (!item->isSelectable() || !item->isVisible()))
			item = item->itemBelow();
	}
	else if( (s == Qt::NoModifier) && (k==Qt::Key_End) ) {
		if (d->currentWidget && d->currentWidget->hasFocus())
			return false;
		//find last visible
		item = selectedItem();
		Q3ListViewItem *lastVisible = item;
		while (item) { // && (!item->isSelectable() || !item->isVisible()))
			item = item->itemBelow();
			if (item && item->isSelectable() && item->isVisible())
				lastVisible = item;
		}
		item = lastVisible;
	}

	if(item) {
		ev->accept();
		ensureItemVisible(item);
		setSelected(item, true);
		return true;
	}
	return false;
}

void
Editor::updateFont()
{
	setFont(parentWidget()->font());
	d->baseRowHeight = QFontMetrics(parentWidget()->font()).height() + itemMargin() * 2;
	if (!d->currentItem)
		d->undoButton->resize(d->baseRowHeight, d->baseRowHeight);
	else {
		showUndoButton(d->undoButton->isVisible());
		updateEditorGeometry();
	}
	updateGroupLabelsPosition();
}

bool
Editor::event( QEvent * e )
{
	if (e->type()==QEvent::FontChange) {
		updateFont();
	}
	return K3ListView::event(e);
}

void
Editor::contentsMousePressEvent( QMouseEvent * e )
{
	Q3ListViewItem *item = itemAt(e->pos());
	if (dynamic_cast<EditorGroupItem*>(item)) {
		setOpen( item, !isOpen(item) );
		return;
	}
	K3ListView::contentsMousePressEvent(e);
}

void
Editor::setSorting( int column, bool ascending )
{
	if (d->set && d->set->groupNames().count()>1) //do not sort when groups are present (maybe reenable this later?)
		return;
	K3ListView::setSorting( column, ascending );
	updateEditorGeometry();
	updateGroupLabelsPosition();
	repaintContents();
	repaint();
}

#include "editor.moc"
