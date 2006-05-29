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

#include "editoritem.h"
#include "editor.h"
#include "property.h"
#include "widget.h"
#include "factory.h"
#include "utils.h"

#include <QPainter>
#include <QPixmap>
#include <q3header.h>
#include <QStyle>
#include <QLabel>
#include <qstyleoption.h>
#include <QStyleOptionHeader>
#include <QApplication>
#include <qlayout.h>

#ifdef QT_ONLY
#else
#include <kdebug.h>
#include <kiconloader.h>
#include <kstyle.h>
#endif

#define BRANCHBOX_SIZE 9

namespace KoProperty {
class EditorItemPrivate
{
	public:
		EditorItemPrivate()
		: property(0) {}
		~EditorItemPrivate() {}

		Property  *property;
		Editor  *editor;
};

//! @internal
static void paintListViewExpander(QPainter* p, QWidget* w, int height, const QColorGroup& cg, bool isOpen)
{
	const int marg = (height -2 - BRANCHBOX_SIZE) / 2;
	int xmarg = marg;
//		if (dynamic_cast<EditorGroupItem*>(item))
//				xmarg = xmarg * 10 / 14 -1;
#if 0 
//! @todo disabled: kstyles do not paint background yet... reenable in the future...
	KStyle* kstyle = dynamic_cast<KStyle*>(widget->style());
	if (kstyle) {
		kstyle->drawKStylePrimitive(
			KStyle::KPE_ListViewExpander, p, w, QRect( xmarg, marg, BRANCHBOX_SIZE, BRANCHBOX_SIZE ), 
			cg, isOpen ? 0 : QStyle::Style_On,
				QStyleOption::Default);
	}
	else {
#endif
		Q_UNUSED(w);
		//draw by hand
		p->setPen( KPROPEDITOR_ITEM_BORDER_COLOR );
		p->drawRect(xmarg, marg, BRANCHBOX_SIZE, BRANCHBOX_SIZE);
		p->fillRect(xmarg+1, marg + 1, BRANCHBOX_SIZE-2, BRANCHBOX_SIZE-2,
//			item->listView()->paletteBackgroundColor());
			cg.base());
//		p->setPen( item->listView()->paletteForegroundColor() );
		p->setPen( cg.foreground() );
		p->drawLine(xmarg+2, marg+BRANCHBOX_SIZE/2, xmarg+BRANCHBOX_SIZE-3, marg+BRANCHBOX_SIZE/2);
		if(!isOpen) {
			p->drawLine(xmarg+BRANCHBOX_SIZE/2, marg+2,
				xmarg+BRANCHBOX_SIZE/2, marg+BRANCHBOX_SIZE-3);
		}
//	}
}

//! @internal
//! Based on KPopupTitle, see kpopupmenu.cpp
class GroupWidgetBase : public QWidget
{
	public:
		GroupWidgetBase(QWidget* parent)
		: QWidget(parent)
		, m_isOpen(true)
		, m_mouseDown(false)
		{
		}

		void setText( const QString &text )
		{
			m_titleStr = text;
		}

		void setIcon( const QPixmap &pix )
		{
			m_miniicon = pix;
		}

		virtual bool isOpen() const
		{
			return m_isOpen;
		}

		virtual void setOpen(bool set)
		{
			m_isOpen = set;
		}

		virtual QSize sizeHint () const
		{
			QSize s( QWidget::sizeHint() );
			s.setHeight(fontMetrics().height()*2);
			return s;
		}

	protected:
		virtual void paintEvent(QPaintEvent *) {
			QRect r(rect());
			QPainter p(this);
			QStyleOptionHeader option;
			option.initFrom(this);
			option.state = m_mouseDown ? QStyle::State_Sunken : QStyle::State_Raised;
			style()->drawControl(QStyle::CE_Header, &option, &p, this);

			paintListViewExpander(&p, this, r.height()+2, palette().active(), isOpen());
			if (!m_miniicon.isNull()) {
				p.drawPixmap(24, (r.height()-m_miniicon.height())/2, m_miniicon);
			}

			if (!m_titleStr.isNull())
			{
				int indent = 16 + (m_miniicon.isNull() ? 0 : (m_miniicon.width()+4));
				p.setPen(palette().active().text());
				QFont f = p.font();
				f.setBold(true);
				p.setFont(f);
				p.drawText(indent+8, 0, width()-(indent+8),
						height(), Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine,
						m_titleStr);
			}
//			p.setPen(palette().active().mid());
//			p.drawLine(0, 0, r.right(), 0);
		}

		virtual bool event( QEvent * e ) {
			if (e->type()==QEvent::MouseButtonPress || e->type()==QEvent::MouseButtonRelease) {
				QMouseEvent* me = static_cast<QMouseEvent*>(e);
				if (me->button() == Qt::LeftButton) {
					m_mouseDown = e->type()==QEvent::MouseButtonPress;
					update();
				}
			}
			return QWidget::event(e);
		}

	protected:
		QString m_titleStr;
		QPixmap m_miniicon;
		bool m_isOpen : 1;
		bool m_mouseDown : 1;
};

class GroupWidget : public GroupWidgetBase
{
	public:
		GroupWidget(EditorGroupItem *parentItem)
		: GroupWidgetBase(parentItem->listView()->viewport())
		, m_parentItem(parentItem)
		{
		}

		virtual bool isOpen() const
		{
			return m_parentItem->isOpen();
		}

	protected:
		EditorGroupItem *m_parentItem;
};

class GroupContainer::Private
{
	public:
		Private() {}
		QVBoxLayout* lyr;
		GroupWidgetBase *groupWidget;
		QPointer<QWidget> contents;
};

}//namespace
using namespace KoProperty;

GroupContainer::GroupContainer(const QString& title, QWidget* parent)
: QWidget(parent)
, d(new Private())
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	d->lyr = new QVBoxLayout(this);
	d->groupWidget = new GroupWidgetBase(this);
	d->groupWidget->setText( title );
	d->lyr->addWidget(d->groupWidget);
	d->lyr->addSpacing(4);
}

GroupContainer::~GroupContainer()
{
	delete d;
}

void GroupContainer::setContents( QWidget* contents )
{
	if (d->contents) {
		d->contents->hide();
		d->lyr->remove(d->contents);
		delete d->contents;
	}
	d->contents = contents;
	if (d->contents) {
		d->lyr->addWidget(d->contents);
		d->contents->show();
	}
	update();
}

bool GroupContainer::event( QEvent * e ) {
	if (e->type()==QEvent::MouseButtonPress) {
		QMouseEvent* me = static_cast<QMouseEvent*>(e);
		if (me->button() == Qt::LeftButton && d->contents && d->groupWidget->rect().contains(me->pos())) {
			d->groupWidget->setOpen(!d->groupWidget->isOpen());
			if (d->groupWidget->isOpen())
				d->contents->show();
			else
				d->contents->hide();
			d->lyr->invalidate();
			update();
		}
	}
	return QWidget::event(e);
}

//////////////////////////////////////////////////////

EditorItem::EditorItem(Editor *editor, EditorItem *parent, Property *property, Q3ListViewItem *after)
 : K3ListViewItem(parent, after,
	property->captionForDisplaying().isEmpty() ? property->name() : property->captionForDisplaying())
{
	d = new EditorItemPrivate();
	d->property = property;
	d->editor = editor;

	setMultiLinesEnabled(true);
	//setHeight(static_cast<Editor*>(listView())->baseRowHeight()*3);
/*
	if (property && !property->caption().isEmpty()) {
			QSimpleRichText srt(property->caption(), font());
			srt.setWidth(columnWidth(0)-KPROPEDITOR_ITEM_MARGIN*2-20+1);
			int oldHeight = it.current()->height();
			int textHeight = srt.height()+KPROPEDITOR_ITEM_MARGIN;
			int textLines = textHeight / d->baseRowHeight + (((textHeight % d->baseRowHeight) > 0) ? 1 : 0);
			kDebug() << " textLines: " << textLines << endl;
			if (textLines != newNumLines) {
				dynamic_cast<EditorItem*>(it.current())->setHeight(newNumLines * d->baseRowHeight);
			}
			kDebug() << it.current()->text(0) << ":  "  << oldHeight << " -> " << newHeight << endl;
		}
*/
}

EditorItem::EditorItem(K3ListView *parent)
 : K3ListViewItem(parent)
{
	d = new EditorItemPrivate();
	d->property = 0;
	d->editor = 0;
	setMultiLinesEnabled(true);
}

EditorItem::EditorItem(EditorItem *parent, const QString &text)
 : K3ListViewItem(parent, text)
{
	d = new EditorItemPrivate();
	d->property = 0;
	d->editor = 0;
	setMultiLinesEnabled(true);
}

EditorItem::EditorItem(EditorItem *parent, EditorItem *after, const QString &text)
 : K3ListViewItem(parent, after, text)
{
	d = new EditorItemPrivate();
	d->property = 0;
	d->editor = 0;
	setMultiLinesEnabled(true);
}

EditorItem::~EditorItem()
{
	delete d;
}

Property*
EditorItem::property()
{
	return d->property;
}

void
EditorItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
	//int margin = static_cast<Editor*>(listView())->itemMargin();
	if(!d->property)
		return;

	if(column == 0)
	{
		QFont font = listView()->font();
		if(d->property->isModified())
			font.setBold(true);
		p->setFont(font);
		p->setBrush(cg.highlight());
		p->setPen(cg.highlightedText());
#ifdef QT_ONLY
		Q3ListViewItem::paintCell(p, cg, column, width, align);
#else
		K3ListViewItem::paintCell(p, cg, column, width, align);
#endif
		p->fillRect(parent() ? 0 : 50, 0, width, height()-1,
			QBrush(isSelected() ? cg.highlight() : backgroundColor()));
		p->setPen(isSelected() ? cg.highlightedText() : cg.text());
		int delta = -20+KPROPEDITOR_ITEM_MARGIN;
		if ((firstChild() && dynamic_cast<EditorGroupItem*>(parent()))) {
			delta = -KPROPEDITOR_ITEM_MARGIN-1;
		}
		if (dynamic_cast<EditorDummyItem*>(parent())) {
			delta = KPROPEDITOR_ITEM_MARGIN*2;
		}
		else if (parent() && dynamic_cast<EditorDummyItem*>(parent()->parent())) {
			if (dynamic_cast<EditorGroupItem*>(parent()))
				delta += KPROPEDITOR_ITEM_MARGIN*2;
			else
				delta += KPROPEDITOR_ITEM_MARGIN*5;
		}
		p->drawText(
			QRect(delta,2, width+listView()->columnWidth(1)-KPROPEDITOR_ITEM_MARGIN*2, height()),
			Qt::AlignLeft | Qt::AlignTop /*| Qt::TextSingleLine*/, text(0));

		p->setPen( KPROPEDITOR_ITEM_BORDER_COLOR );
		p->drawLine(width-1, 0, width-1, height()-1);
		p->drawLine(0, -1, width-1, -1);

		p->setPen( KPROPEDITOR_ITEM_BORDER_COLOR ); //! \todo custom color?
		if (dynamic_cast<EditorDummyItem*>(parent()))
			p->drawLine(0, 0, 0, height()-1 );
	}
	else if(column == 1)
	{
		QColorGroup icg(cg);
#ifdef QT_ONLY
		icg.setColor(QColorGroup::Background, cg.base());
#else
		icg.setColor(QColorGroup::Background, backgroundColor());
		p->setBackgroundColor(backgroundColor());
#endif
		Widget *widget = d->editor->createWidgetForProperty(d->property, false /*don't change Widget::property() */);
		if(widget) {
			QRect r(0, 0, d->editor->header()->sectionSize(1), height() - (widget->hasBorders() ? 0 : 1));
			p->setClipRect(r);
			p->setClipping(true);
			widget->drawViewer(p, icg, r, d->property->value());
			p->setClipping(false);
		}
	}
	p->setPen( KPROPEDITOR_ITEM_BORDER_COLOR ); //! \todo custom color?
	p->drawLine(0, height()-1, width, height()-1 );
}

void
EditorItem::paintBranches(QPainter *p, const QColorGroup &cg, int w, int y, int h)
{
	p->eraseRect(0,0,w,h);
#ifdef QT_ONLY
	Q3ListViewItem *item = firstChild();
#else
	K3ListViewItem *item = static_cast<K3ListViewItem*>(firstChild());
#endif
	if(!item)
		return;

	QColor backgroundColor;
	p->save();
	p->translate(0,y);
	QFont font = listView()->font();
	while(item)
	{
		if(item->isSelected())
			backgroundColor = cg.highlight();
		else {
#ifdef QT_ONLY
			backgroundColor = cg.base();
#else
			if (dynamic_cast<EditorGroupItem*>(item))
				backgroundColor = cg.base();
			else
				backgroundColor = item->backgroundColor();
#endif
		}
//		p->fillRect(-50,0,50, item->height(), QBrush(backgroundColor));
		p->save();
		p->setPen( KPROPEDITOR_ITEM_BORDER_COLOR );
		int delta = 0;
		int fillWidth = w;
		int x = 0;
		if (dynamic_cast<EditorGroupItem*>(item->parent())) {
			delta = 0;//-19;
			fillWidth += 19;
		}
		else {
			if (dynamic_cast<EditorGroupItem*>(item) || /*for flat mode*/ dynamic_cast<EditorDummyItem*>(item->parent()))
				x = 19;
			else
				x = -19;
			fillWidth += 19;
		}
		if (dynamic_cast<EditorDummyItem*>(item->parent())) {
			x = 19;
		}
		else if (item->parent() && dynamic_cast<EditorDummyItem*>(item->parent()->parent())) {
			x = 0;
		}
		p->fillRect(x+1, 0, fillWidth-1, item->height()-1, QBrush(backgroundColor));
		p->drawLine(x, item->height()-1, w, item->height()-1 );
		if (!dynamic_cast<EditorGroupItem*>(item))
			p->drawLine(x, 0, x, item->height()-1 );
		p->restore();

//	for (int i=0; i<10000000; i++)
//		;
//		if(item->isSelected())  {
//			p->fillRect(parent() ? 0 : 50, 0, w, item->height()-1, QBrush(cg.highlight()));
//			p->fillRect(-50,0,50, item->height(), QBrush(cg.highlight()));
//		}

		//sorry, but we need to draw text here again
		font.setBold( dynamic_cast<EditorGroupItem*>(item)
			|| (static_cast<EditorItem*>(item)->property() && static_cast<EditorItem*>(item)->property()->isModified()) );
		p->setFont(font);
		p->setPen(item->isSelected() ? cg.highlightedText() : cg.text());
		if (item->firstChild() && dynamic_cast<EditorGroupItem*>(item->parent())) {
			delta = 19-KPROPEDITOR_ITEM_MARGIN-1;
		}
		else if (dynamic_cast<EditorDummyItem*>(item->parent())) {
			delta = 19;
		}
		if (item->parent() && dynamic_cast<EditorDummyItem*>(item->parent()->parent())) {
			if (dynamic_cast<EditorGroupItem*>(item->parent()))
				delta += KPROPEDITOR_ITEM_MARGIN*2;
			else
				delta += KPROPEDITOR_ITEM_MARGIN*5;
		}

		if (!dynamic_cast<EditorDummyItem*>(item->parent()))
			p->drawText(QRect(delta+1,0, w+listView()->columnWidth(1), item->height()),
			Qt::AlignLeft | Qt::AlignVCenter /*| Qt::TextSingleLine*/, item->text(0));

		if(item->firstChild())  {
			paintListViewExpander(p, listView(), item->height(),
				cg, item->isOpen());
		}

		// draw icon (if there is one)
		EditorItem *editorItem = dynamic_cast<EditorItem*>(item);
		if (editorItem && editorItem->property() && !editorItem->property()->icon().isEmpty()) {
			//int margin = listView()->itemMargin();
			QPixmap pix = SmallIcon(editorItem->property()->icon());
			if (!pix.isNull())
				p->drawPixmap(-19+(19-pix.width())/2, (item->height() - pix.height()) / 2, pix);
		}

		p->translate(0, item->totalHeight());
#ifdef QT_ONLY
		item = item->nextSibling();
#else
		item = (K3ListViewItem*)item->nextSibling();
#endif
	}
	p->restore();
}

void
EditorItem::paintFocus(QPainter *, const QColorGroup &, const QRect & )
{
}

int
EditorItem::compare( Q3ListViewItem *i, int col, bool ascending ) const
{
	if (!ascending)
		return -Q3ListViewItem::key( col, ascending ).localeAwareCompare( i->key( col, ascending ) );

	if (d->property) {
//		kopropertydbg << d->property->name() << " " << d->property->sortingKey() << " | "
//			<< static_cast<EditorItem*>(i)->property()->name() << " "
//			<< static_cast<EditorItem*>(i)->property()->sortingKey() << endl;
		return d->property->sortingKey()
			- ((dynamic_cast<EditorItem*>(i) && dynamic_cast<EditorItem*>(i)->property()) 
				? dynamic_cast<EditorItem*>(i)->property()->sortingKey() : 0);
	}

	return 0;
//	return d->order - static_cast<EditorItem*>(i)->d->order;
}

void
EditorItem::setHeight( int height )
{
	K3ListViewItem::setHeight(height);
}

//////////////////////////////////////////////////////

EditorGroupItem::EditorGroupItem(EditorItem *parent, EditorItem *after, const QString &text, const QString &icon, int sortOrder)
 : EditorItem(parent, after, text)
 , m_label(0)
 , m_sortOrder(sortOrder)
{
	init(icon);
}

EditorGroupItem::EditorGroupItem(EditorItem *parent, const QString &text, const QString &icon, int sortOrder)
 : EditorItem(parent, text)
 , m_label(0)
 , m_sortOrder(sortOrder)
{
	init(icon);
}

EditorGroupItem::~EditorGroupItem()
{
	delete m_label;
}

QWidget* EditorGroupItem::label() const
{
	return m_label;
}

void EditorGroupItem::init(const QString &icon)
{
	setOpen(true);
	setSelectable(false);
	m_label = new GroupWidget(this);
	m_label->setText(text(0)); //todo: icon?
	if (!icon.isEmpty())
		m_label->setIcon( SmallIcon(icon) );
	m_label->show();
}

void
EditorGroupItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int /*align*/)
{
	Q_UNUSED(p);
	Q_UNUSED(cg);
	Q_UNUSED(column);
	Q_UNUSED(width);
	//no need to draw anything since there's a label on top of it
//	p->fillRect(0, 0, width, height(), cg.base());

	//if(column == 1)
	//	return;
	/*p->setPen( KPROPEDITOR_ITEM_BORDER_COLOR ); //! \todo custom color?

	p->setClipRect(listView()->itemRect(this));
	if(column == 1)
		p->translate(-listView()->columnWidth(0) + 20, 0);
	int totalWidth = listView()->columnWidth(0) + listView()->columnWidth(1) - 20;
	p->eraseRect(QRect(0,0, totalWidth,height()-1));
	p->drawLine(0, height()-1, totalWidth-1, height()-1);

	QFont font = listView()->font();
	font.setBold(true);
	p->setFont(font);
	p->setBrush(cg.highlight());
	//p->setPen(cg.highlightedText());
#ifdef QT_ONLY
		QListViewItem::paintCell(p, cg, column, width, align);
#else
		K3ListViewItem::paintCell(p, cg, column, width, align);
#endif
	p->setPen(cg.text());
	p->drawText(QRect(0,0, totalWidth, height()),
		Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, text(0));*/
}

void
EditorGroupItem::setup()
{
	K3ListViewItem::setup();
	setHeight( height()+4 );
}

int
EditorGroupItem::compare( Q3ListViewItem *i, int col, bool ascending ) const
{
	Q_UNUSED(col);
	Q_UNUSED(ascending);
	if (dynamic_cast<EditorGroupItem*>(i)) {
		return m_sortOrder 
			- dynamic_cast<EditorGroupItem*>(i)->m_sortOrder;
	}
	return 0;
}

////////////////////////////////////////////////////////

EditorDummyItem::EditorDummyItem(K3ListView *listview)
 : EditorItem(listview)
{
	setSelectable(false);
	setOpen(true);
}

EditorDummyItem::~EditorDummyItem()
{}

void
EditorDummyItem::setup()
{
	setHeight(0);
}
