/* This file is part of the KDE project
 *
 * Copyright (c) 2008 Casper Boemann <cbr@boemann.dk>
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
#include "KoDockerManager.h"

#include <klocale.h>

#include <KoView.h>
#include <KoToolDockerFactory.h>
#include <KoToolDocker.h>

class KoDockerManager::Private {
public:
    Private() : view(0) {}
    KoView *view;
    KoToolDocker* toolDocker[10];
};


KoDockerManager::KoDockerManager(KoView *view)
    :  d( new Private() )
{
    d->view = view;
    KoToolDockerFactory toolDockerFactory;
    d->toolDocker[0] = qobject_cast<KoToolDocker*>(d->view->createDockWidget(&toolDockerFactory));
    d->toolDocker[1] = qobject_cast<KoToolDocker*>(d->view->createDockWidget(&toolDockerFactory));
}

KoDockerManager::~KoDockerManager()
{
    delete d;
}

void KoDockerManager::newOptionWidgets(const QMap<QString, QWidget *> & optionWidgetMap)
{
    int count=0;
    QMapIterator<QString, QWidget *> i(optionWidgetMap);
    while (i.hasNext()) {
        i.next();
        d->toolDocker[count]->newOptionWidget( i.value() );
        d->toolDocker[count]->setWindowTitle( i.key() );
        d->toolDocker[count]->show();
        count++;
    }
    while(count<2)
        d->toolDocker[count++]->hide();
}

#include "KoDockerManager.moc"
