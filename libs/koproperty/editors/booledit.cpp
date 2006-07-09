/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>

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

#ifndef QT_ONLY
#include <kiconloader.h>
#include <klocale.h>
#endif

#include <QToolButton>
#include <QPainter>
#include <QVariant>
#include <QLayout>
//Added by qt3to4:
#include <QResizeEvent>
#include <QKeyEvent>
#include <QEvent>

#include <kdebug.h>

using namespace KoProperty;

BoolEdit::BoolEdit(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
{
    m_toggle = new QToolButton(this);
    m_toggle->setFocusPolicy(Qt::WheelFocus);
    m_toggle->setCheckable(true);
    m_toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //we're not using layout to because of problems with button size
    m_toggle->move(0, 0);
    m_toggle->resize(width(), height());
//    m_toggle->show();

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

void
BoolEdit::drawViewer(QPainter *p, const QColorGroup &, const QRect &r, const QVariant &value)
{
    p->eraseRect(r);
    QRect r2(r);
    r2.moveLeft(K3Icon::SizeSmall + 6);

    if(value.toBool()) {
        p->drawPixmap(3, (r.height()-1-K3Icon::SizeSmall)/2, SmallIcon("button_ok"));
        p->drawText(r2, Qt::AlignVCenter | Qt::AlignLeft, i18n("Yes"));
    }
    else  {
        p->drawPixmap(3, (r.height()-1-K3Icon::SizeSmall)/2, SmallIcon("button_no"));
        p->drawText(r2, Qt::AlignVCenter | Qt::AlignLeft, i18n("No"));
    }
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
        if(ev->key() == Qt::Key_Space) {
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

#include "booledit.moc"
