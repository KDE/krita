/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
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

#include "ActionSearchLine.h"

#include <QDebug>
#include <QPushButton>
#include <QFrame>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QAction>
#include <QVariant>

#include <kis_global.h>

#include "ActionSearchWidget.h"

class ActionSearchLine::Private
{
public:
    bool popupVisible {false};
    QFrame *frame {0};
    ActionSearchWidget *searchWidget {0};
    QHBoxLayout *frameLayout {0};
};

ActionSearchLine::ActionSearchLine(KActionCollection *actionCollection, QWidget *parent)
    : QLineEdit(parent)
    , d(new ActionSearchLine::Private())
{
    d->frame = new QFrame(this);
    d->searchWidget = new ActionSearchWidget(actionCollection, this);
    connect(d->searchWidget, SIGNAL(actionTriggered()), SLOT(hidePopup()));
    d->frame->setFrameStyle(QFrame::Box |  QFrame::Plain);
    d->frame->setWindowFlags(Qt::Popup);
    d->frameLayout = new QHBoxLayout(d->frame);
    d->frameLayout->setMargin(0);
    d->frameLayout->setSizeConstraint(QLayout::SetFixedSize);
    d->frame->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    d->searchWidget->setParent(d->frame);
    d->frameLayout->addWidget(d->searchWidget);
    d->frame->setFrameStyle(Qt::Popup);
}

ActionSearchLine::~ActionSearchLine()
{
}

void ActionSearchLine::showPopup()
{
    if (d->searchWidget && !d->searchWidget->isVisible()) {
        d->frame->raise();
        d->frame->show();
        adjustPosition();
    }
    else {
        hidePopup();
    }
}

void ActionSearchLine::hidePopup()
{
    if (d->searchWidget) {
        d->frame->setVisible(false);
    }
}

void ActionSearchLine::focusInEvent(QFocusEvent *ev)
{
    QLineEdit::focusInEvent(ev);
    showPopup();
}

void ActionSearchLine::adjustPosition()
{
    QSize popSize = d->searchWidget->size();
    QRect popupRect(this->mapToGlobal(QPoint(0, this->size().height())), popSize);

    // Get the available geometry of the screen which contains this KisPopupButton
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->availableGeometry(this);
    popupRect = kisEnsureInRect(popupRect, screenRect);

    d->frame->setGeometry(popupRect);
}
