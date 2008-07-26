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
#include <KoMainWindow.h>

#include <QAction>

class KoDockerManager::Private {
public:
    Private() : view(0) {}
    KoView *view;
    KoToolDocker* toolDocker[10];
};

#define NUMDOCKERS 3
KoDockerManager::KoDockerManager(KoView *view)
    : QObject(view), d( new Private() )
{
    d->view = view;
    KoToolDockerFactory toolDockerFactory;
    for (int i=0; i < NUMDOCKERS; i++) {
        d->toolDocker[i] = qobject_cast<KoToolDocker*>(d->view->createDockWidget(&toolDockerFactory));
    }
    connect(view->shell(), SIGNAL(restoringDone()), this, SLOT(removeUnusedOptionWidgets()));

}

KoDockerManager::~KoDockerManager()
{
    delete d;
}

void KoDockerManager::removeUnusedOptionWidgets()
{
    for (int i=0; i < NUMDOCKERS; i++) {
        if (!d->toolDocker[i]->hasOptionWidget()) {
            d->view->removeDockWidget(d->toolDocker[i]);
        }
    }
}

void KoDockerManager::newOptionWidgets(const QMap<QString, QWidget *> & optionWidgetMap)
{
    int count=0;
    QMapIterator<QString, QWidget *> i(optionWidgetMap);
    while (i.hasNext()) {
        i.next();
        d->toolDocker[count]->removeOptionWidget();
        d->toolDocker[count]->newOptionWidget( i.value() );
        d->toolDocker[count]->setWindowTitle( i.key() );
        d->view->restoreDockWidget(d->toolDocker[count]);
        d->toolDocker[count]->toggleViewAction()->setVisible(true);
        count++;
    }
    while(count < NUMDOCKERS) {
        d->toolDocker[count]->removeOptionWidget();
        d->toolDocker[count]->toggleViewAction()->setVisible(false);
        d->view->removeDockWidget(d->toolDocker[count]);
        count++;
    }
}

#include "KoDockerManager.moc"
