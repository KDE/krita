/* This file is part of the KDE project
 * Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoColorSetAction.h"

#include "KoColorSetWidget.h"

#include <QWidgetAction>
#include <QMenu>

#include <klocale.h>
#include <kicon.h>

class KoColorSetAction::KoColorSetActionPrivate
{
public:
    KoColorSetWidget *colorSetWidget;
    QMenu *menu;
};

KoColorSetAction::KoColorSetAction(QObject *parent)
    : QAction(parent),
    d(new KoColorSetActionPrivate())
{
    d->menu = new QMenu();
    QWidgetAction *wdgAction = new QWidgetAction(d->menu);
    d->colorSetWidget = new KoColorSetWidget(d->menu);
    connect(d->colorSetWidget, SIGNAL(colorChanged(const KoColor &, bool)), this, SLOT(handleColorChange(const KoColor &, bool)));
    wdgAction->setDefaultWidget(d->colorSetWidget);
    d->menu->addAction(wdgAction);
    setMenu(d->menu);
    setIcon(KIcon("textcolor"));
    setToolTip(i18n("Text Color..."));
}

KoColorSetAction::~KoColorSetAction()
{
    delete d;
}

void KoColorSetAction::handleColorChange(const KoColor &color, bool final)
{
    if (final) {
        menu()->hide();
        emit colorChanged(color);
    }
}

#include "KoColorSetAction.moc"
