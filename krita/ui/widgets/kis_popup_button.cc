/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "widgets/kis_popup_button.h"

#include <QApplication>
#include <QDesktopWidget>

#include <QFrame>
#include <QHBoxLayout>

#include <kis_debug.h>

struct KisPopupButton::Private {
    Private() : frame(0), frameLayout(0), popupWidget(0) {}
    QFrame* frame;
    QHBoxLayout* frameLayout;
    QWidget* popupWidget;
};

KisPopupButton::KisPopupButton(QWidget* parent) : QPushButton(parent), d(new Private)
{
    d->frame = new QFrame();
    d->frame->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->frame->setWindowFlags(Qt::Popup);
    d->frameLayout = new QHBoxLayout(d->frame);
    d->frameLayout->setMargin(0);
    setPopupWidget(new QWidget);
    connect(this, SIGNAL(released()), SLOT(showPopupWidget()));
}

KisPopupButton::~KisPopupButton()
{
    delete d->frame;
    delete d;
}

void KisPopupButton::setAlwaysVisible(bool v)
{
    if (v) {
        d->frame->setFrameStyle(Qt::SubWindow);
        showPopupWidget();
    } else {
        d->frame->setFrameStyle(Qt::Popup);
    }
}

void KisPopupButton::setPopupWidget(QWidget* widget)
{
    delete d->popupWidget;
    if (widget) {
        d->popupWidget = widget;
        d->popupWidget->setParent(d->frame);
    } else {
        d->popupWidget = new QWidget(d->frame);
    }
    d->frameLayout->addWidget(d->popupWidget);
}

void KisPopupButton::setPopupWidgetWidth(int w)
{
    d->frame->resize(w, d->frame->height());
}

void KisPopupButton::showPopupWidget()
{
    if (d->popupWidget) {
        QSize popSize = d->frame->size();
        QRect popupRect(this->mapToGlobal(QPoint(x(), this->size().height())), popSize);

        // Make sure the popup is not drawn outside the screen area
        QRect screenRect = QApplication::desktop()->availableGeometry(d->frame);
        if (popupRect.right() > screenRect.right())
            popupRect.translate(screenRect.right() - popupRect.right(), 0);
        if (popupRect.left() < screenRect.left())
            popupRect.translate(screenRect.left() - popupRect.left(), 0);
        if (popupRect.bottom() > screenRect.bottom())
            popupRect.translate(0, -d->frame->height());

        d->frame->setGeometry(popupRect);
        d->frame->raise();
        d->frame->show();

    }
}

void KisPopupButton::hidePopupWidget()
{
    if (d->popupWidget) {
        d->frame->setVisible(false);
    }
}

#include "kis_popup_button.moc"
