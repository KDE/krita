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

#include "kis_popup_button.h"

#include <kdebug.h>

struct KisPopupButton::Private {
    Private() : popupWidget(new QWidget) {}
    QWidget* popupWidget;
};

KisPopupButton::KisPopupButton(QWidget* parent) : QPushButton(parent), d(new Private)
{
    connect(this, SIGNAL(released()), SLOT(showPopupWidget()));
}

KisPopupButton::~KisPopupButton()
{
    delete d->popupWidget;
    delete d;
}

void KisPopupButton::setPopupWidget(QWidget* widget)
{
    delete d->popupWidget;
    d->popupWidget = widget;
    d->popupWidget->setWindowFlags(Qt::Popup);
}

void KisPopupButton::showPopupWidget()
{
    d->popupWidget->move( mapToGlobal ( QPoint(0, height() )));
    d->popupWidget->setVisible(true);
}

#include "kis_popup_button.moc"
