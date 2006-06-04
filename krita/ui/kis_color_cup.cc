/*
 * This file is part of Krita
 *
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QPushButton>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <qdrawutil.h>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QSpinBox>
#include <QStyle>
#include <QToolTip>
#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>

#include <kcolordialog.h>
#include <klocale.h>
#include <knuminput.h>
#include <koFrameButton.h>

#include <kis_canvas_subject.h>
#include <KoColor.h>
#include <kis_color_cup.h>

KoColorPopup::KoColorPopup(QColor c, QWidget * parent, const char * name)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setObjectName(name);
    m_color = c;
    setContentsMargins(4, 4, 4, 4);
    setFocusPolicy(Qt::StrongFocus);
    QHBoxLayout * l  = new QHBoxLayout(this);
    l->addWidget(m_khsSelector = new KHSSelector(this));
    m_khsSelector->setMinimumSize(140, 7);
    l->addWidget(m_valueSelector = new KValueSelector(this));
    m_valueSelector->setMinimumSize(26, 70);
    m_khsSelector->show();
    m_valueSelector->show();

}

KoColorCup::KoColorCup(QWidget * parent, const char * name)
    : QPushButton(parent)
{
    setObjectName(name);
    m_color = Qt::black;
    m_popup = new KoColorPopup(m_color, this, "colorpopup");
    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(m_popup, SIGNAL(changed( const QColor &)), this, SLOT(setColor(const QColor &)));
}

void KoColorCup::setColor(const QColor & c)
{
    m_color = c;
    emit changed(c);
}

void KoColorCup::slotClicked()
{    
//    m_popup->move(this->mapToGlobal( this->rect().topRight() ) );
//    m_popup->show();
    emit changed(m_color);
}

QSize KoColorCup::sizeHint() const
{
    QStyleOptionButton option;
    option.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_PushButton, &option, QSize(24, 24), this).
	  	expandedTo(QApplication::globalStrut());
}

void KoColorCup::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QStyleOptionButton option;
    option.initFrom(this);

    style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);

    int x, y, w, h;
    QRect r = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    r.getRect(&x, &y, &w, &h);

    int margin = 3; //style().pixelMetric( QStyle::PM_ButtonMargin, this );
    x += margin;
    y += margin;
    w -= 2*margin;
    h -= 2*margin;

    if (isChecked() || isDown()) {
        x += style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal);
        y += style()->pixelMetric(QStyle::PM_ButtonShiftVertical);
    }

    qDrawShadePanel(&painter, x, y, w, h, palette(), true, 1, NULL);
    if (m_color.isValid())
        painter.fillRect(x + 1, y + 1, w - 2, h - 2, m_color);

    if (hasFocus()) {
        QRect focusRect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &option, this);

        QStyleOptionFocusRect optionFocusRect;
        optionFocusRect.initFrom(this);
        optionFocusRect.rect = focusRect;
        optionFocusRect.backgroundColor = palette().background().color();

        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &optionFocusRect, &painter, this);
    }
}

#include "kis_color_cup.moc"
