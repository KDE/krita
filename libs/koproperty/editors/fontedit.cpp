/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
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
 * Boston, MA 02110-1301, USA.
*/

#include "fontedit.h"
#include "editoritem.h"

#include <QPushButton>
#include <QPainter>
#include <QLayout>
#include <QVariant>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>
#include <QToolTip>
//Added by qt3to4:
#include <QEvent>
#include <QKeyEvent>
#include <Q3Frame>
#include <QResizeEvent>

#include <kdeversion.h>
#include <kfontrequester.h>
#include <kacceleratormanager.h>

#ifdef QT_ONLY
//! \todo
#else
#include <klocale.h>
#endif

//! @internal
//! reimplemented to better button and label's positioning
class FontEditRequester : public KFontRequester
{
	public:
		FontEditRequester(QWidget* parent)
			: KFontRequester(parent)
		{
			label()->setPaletteBackgroundColor(palette().active().base());
			label()->setMinimumWidth(0);
			label()->setFrameShape(Q3Frame::Box);
			label()->setIndent(-1);
#if KDE_VERSION >= KDE_MAKE_VERSION(3,4,0) 
			label()->setFocusPolicy(Qt::ClickFocus);
			KAcceleratorManager::setNoAccel(label());
#endif
			layout()->remove(label());
			layout()->remove(button());//->reparent(this, 0, QPoint(0,0));
			delete layout();
			button()->setText(i18n("..."));
			button()->setToolTip( i18n("Change font"));
			button()->setFocusPolicy(Qt::NoFocus);
			button()->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			QFontMetrics fm(button()->font());
			button()->setFixedWidth(fm.width(button()->text()+" "));
		}
		virtual void resizeEvent(QResizeEvent *e)
		{
			KFontRequester::resizeEvent(e);
			label()->move(0,0);
			label()->resize(e->size()-QSize(button()->width(),-1));
			button()->move(label()->width(),0);
			button()->setFixedSize(button()->width(), height());
		}
};

using namespace KoProperty;

FontEdit::FontEdit(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
{
	m_edit = new FontEditRequester(this);
	m_edit->setMinimumHeight(5);
	setEditor(m_edit);
	setFocusWidget(m_edit->label());
	connect(m_edit, SIGNAL(fontSelected(const QFont& )), this, SLOT(slotValueChanged(const QFont&)));
}

FontEdit::~FontEdit()
{}

QVariant
FontEdit::value() const
{
	return m_edit->font();
}

static QString sampleText(const QVariant &value)
{
	QFontInfo fi(value.value<QFont>());
	return fi.family() + (fi.bold() ? " " + i18n("Bold") : QString("")) +
		(fi.italic() ? " " + i18n("Italic") : QString::null) +
		" " + QString::number(fi.pointSize());
}

void
FontEdit::setValue(const QVariant &value, bool emitChange)
{
	m_edit->blockSignals(true);
	m_edit->setFont(value.value<QFont>());
	m_edit->blockSignals(false);
	m_edit->setSampleText(sampleText(value));
	if (emitChange)
		emit valueChanged(this);
}

void
FontEdit::drawViewer(QPainter *p, const QColorGroup &, const QRect &r, const QVariant &value)
{
	p->eraseRect(r);
	p->setFont(value.value<QFont>());
	QRect r2(r);
	r2.setLeft(r2.left()+KPROPEDITOR_ITEM_MARGIN);
	r2.setBottom(r2.bottom()+1);
	p->drawText(r2, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, sampleText(value));
}

void
FontEdit::slotValueChanged(const QFont &)
{
	emit valueChanged(this);
}

bool
FontEdit::eventFilter(QObject* watched, QEvent* e)
{
	if(e->type() == QEvent::KeyPress) {
		QKeyEvent* ev = static_cast<QKeyEvent*>(e);
		if(ev->key() == Qt::Key_Space) {
			m_edit->button()->animateClick();
			return true;
		}
	}
	return Widget::eventFilter(watched, e);
}

void
FontEdit::setReadOnlyInternal(bool readOnly)
{
	setVisibleFlag(!readOnly);
}

#include "fontedit.moc"
