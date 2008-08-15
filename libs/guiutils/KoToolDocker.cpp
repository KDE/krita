/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoToolDocker.h"

#include <klocale.h>
#include <QVBoxLayout>

class KoToolDocker::Private {
public:
    Private() : currentWidget(0) {}
    QWidget *currentWidget;
    QWidget *minimalWidget; //used to force the docker to a minimal size
};


KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget("Tool Options initial name - never seen", parent),
    d( new Private() )
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    d->currentWidget = 0;
    d->minimalWidget = new QWidget();
    d->minimalWidget->setMaximumSize(0,0);
    setWidget(d->minimalWidget);
    d->minimalWidget->show();
}

KoToolDocker::~KoToolDocker() {
    delete d;
}

bool KoToolDocker::hasOptionWidget()
{
    return  d->currentWidget != 0;
}

void KoToolDocker::removeOptionWidget()
{
    if(d->currentWidget) {
        d->currentWidget->hide();
        d->currentWidget->setParent(0);
    }
    setWidget(d->minimalWidget);
    d->minimalWidget->show();
    d->currentWidget = 0;
}

void KoToolDocker::newOptionWidget(QWidget *optionWidget) {
    if(d->currentWidget == optionWidget)
        return;
    if(d->currentWidget) {
        d->currentWidget->hide();
        d->currentWidget->setParent(0);
    }
    d->currentWidget = optionWidget;
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // will be overwritten again next
    setWidget(optionWidget);
    d->minimalWidget->setParent(0);
    adjustSize();
    optionWidget->show();
    update(); // force qt to update the layout even when we are floating
}

#include "KoToolDocker.moc"
