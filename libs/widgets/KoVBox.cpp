/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoVBox.h"

#include <QtCore/QEvent>
#include <QApplication>
#include <QVBoxLayout>

KoVBox::KoVBox(QWidget *parent)
    : QFrame(parent),
      d(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    setLayout(layout);
}

KoVBox::~KoVBox()
{
}

void KoVBox::childEvent(QChildEvent *event)
{
    switch (event->type()) {
    case QEvent::ChildAdded: {
        QChildEvent *childEvent = static_cast<QChildEvent *>(event);
        if (childEvent->child()->isWidgetType()) {
            QWidget *widget = static_cast<QWidget *>(childEvent->child());
            static_cast<QBoxLayout *>(layout())->addWidget(widget);
        }

        break;
    }
    case QEvent::ChildRemoved: {
        QChildEvent *childEvent = static_cast<QChildEvent *>(event);
        if (childEvent->child()->isWidgetType()) {
            QWidget *widget = static_cast<QWidget *>(childEvent->child());
            static_cast<QBoxLayout *>(layout())->removeWidget(widget);
        }

        break;
    }
    default:
        break;
    }
    QFrame::childEvent(event);
}

QSize KoVBox::sizeHint() const
{
    KoVBox *that = const_cast<KoVBox *>(this);
    QApplication::sendPostedEvents(that, QEvent::ChildAdded);

    return QFrame::sizeHint();
}

QSize KoVBox::minimumSizeHint() const
{
    KoVBox *that = const_cast<KoVBox *>(this);
    QApplication::sendPostedEvents(that, QEvent::ChildAdded);

    return QFrame::minimumSizeHint();
}

void KoVBox::setSpacing(int spacing)
{
    layout()->setSpacing(spacing);
}

void KoVBox::setStretchFactor(QWidget *widget, int stretch)
{
    static_cast<QBoxLayout *>(layout())->setStretchFactor(widget, stretch);
}

void KoVBox::setMargin(int margin)
{
    layout()->setMargin(margin);
}

