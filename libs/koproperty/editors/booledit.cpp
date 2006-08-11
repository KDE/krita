/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2006 Jaroslaw Staniek <js@iidea.pl>

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

#include "booledit.h"
#include "property.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kdebug.h>

#include <QToolButton>
#include <QPainter>
#include <QVariant>
#include <QLayout>
#include <QBitmap>

using namespace KoProperty;

BoolEdit::BoolEdit(Property *property, QWidget *parent)
 : Widget(property, parent)
 , m_yesIcon( SmallIcon("button_ok") )
 , m_noIcon( SmallIcon("button_no") )
{
    m_toggle = new QToolButton(this);
    m_toggle->setFocusPolicy(Qt::WheelFocus);
    m_toggle->setCheckable(true);
    m_toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //we're not using layout to because of problems with button size
    m_toggle->move(0, 0);
    m_toggle->resize(width(), height());

    setFocusWidget(m_toggle);
    connect(m_toggle, SIGNAL(toggled(bool)), this, SLOT(slotValueChanged(bool)));
}

BoolEdit::~BoolEdit()
{
}

QVariant
BoolEdit::value() const
{
    return QVariant(m_toggle->isChecked());
}

void
BoolEdit::setValue(const QVariant &value, bool emitChange)
{
    m_toggle->blockSignals(true);
    m_toggle->setChecked(value.toBool());
    setState(value.toBool());
    m_toggle->blockSignals(false);
    if (emitChange)
        emit valueChanged(this);
}

void
BoolEdit::slotValueChanged(bool state)
{
    setState(state);
    emit valueChanged(this);
}

static void drawViewerInternal(QPainter *p, const QRect &r, const QVariant &value,
 const QPixmap& yesIcon, const QPixmap& noIcon, const QString& nullText)
{
    p->eraseRect(r);
    QRect r2(r);
    r2.moveLeft(K3Icon::SizeSmall + 6);

    if(value.isNull() && !nullText.isEmpty()) {
        p->drawText(r2, Qt::AlignVCenter | Qt::AlignLeft, nullText);
    }
    else if(value.toBool()) {
        p->drawPixmap(3, (r.height()-1-K3Icon::SizeSmall)/2, yesIcon);
        p->drawText(r2, Qt::AlignVCenter | Qt::AlignLeft, i18n("Yes"));
    }
    else {
        p->drawPixmap(3, (r.height()-1-K3Icon::SizeSmall)/2, noIcon);
        p->drawText(r2, Qt::AlignVCenter | Qt::AlignLeft, i18n("No"));
    }
}

void
BoolEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    Q_UNUSED(cg);
    drawViewerInternal(p, r, value, m_yesIcon, m_noIcon, "");
}

void
BoolEdit::setState(bool state)
{
    if(state)
    {
        m_toggle->setIcon(QIcon(SmallIcon("button_ok")));
        m_toggle->setText(i18n("Yes"));
    }
    else
    {
        m_toggle->setIcon(QIcon(SmallIcon("button_no")));
        m_toggle->setText(i18n("No"));
    }
}

void
BoolEdit::resizeEvent(QResizeEvent *ev)
{
    m_toggle->resize(ev->size());
}

bool
BoolEdit::eventFilter(QObject* watched, QEvent* e)
{
    if(e->type() == QEvent::KeyPress) {
        QKeyEvent* ev = static_cast<QKeyEvent*>(e);
        const int k = ev->key();
        if(k == Qt::Key_Space || k == Qt::Key_Enter || k == Qt::Key_Return) {
            if (m_toggle)
                m_toggle->toggle();
            return true;
        }
    }
    return Widget::eventFilter(watched, e);
}

void
BoolEdit::setReadOnlyInternal(bool readOnly)
{
	setVisibleFlag(!readOnly);
}

//--------------------------------------------------

ThreeStateBoolEdit::ThreeStateBoolEdit(Property *property, QWidget *parent)
 : ComboBox(property, parent)
 , m_yesIcon( SmallIcon("button_ok") )
 , m_noIcon( SmallIcon("button_no") )
{
	m_edit->addItem( m_yesIcon, i18n("Yes") );
	m_edit->addItem( m_noIcon, i18n("No") );
	QVariant thirdState = property ? property->option("3rdState") : QVariant();
	QPixmap nullIcon( m_yesIcon.size() ); //transparent pixmap of appropriate size
	nullIcon.fill( Qt::transparent );
	m_edit->addItem( nullIcon, thirdState.toString().isEmpty() ? i18n("None") : thirdState.toString() );
}

ThreeStateBoolEdit::~ThreeStateBoolEdit()
{
}

QVariant
ThreeStateBoolEdit::value() const
{
	// list items: true, false, NULL
	const int idx = m_edit->currentIndex();
	if (idx==0)
		return QVariant(true);
	else
		return idx==1 ? QVariant(false) : QVariant();
}

void
ThreeStateBoolEdit::setProperty(Property *prop)
{
	m_setValueEnabled = false; //setValue() couldn't be called before fillBox()
	Widget::setProperty(prop);
	m_setValueEnabled = true;
	if(prop)
		setValue(prop->value(), false); //now the value can be set
}

void
ThreeStateBoolEdit::setValue(const QVariant &value, bool emitChange)
{
	if (!m_setValueEnabled)
		return;

	if (value.isNull())
		m_edit->setCurrentIndex(2);
	else
		m_edit->setCurrentIndex(value.toBool() ? 0 : 1);

	if (emitChange)
		emit valueChanged(this);
}

void
ThreeStateBoolEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
	Q_UNUSED(cg);
	drawViewerInternal(p, r, value, m_yesIcon, m_noIcon, m_edit->itemText(2));
}

#include "booledit.moc"
