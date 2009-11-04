/* This file is part of the KDE project
 *
 * Copyright (c) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include "KoToolDocker_p.h"

#include "KoView.h"
#include "KoMainWindow.h"

class KoDockerManager::Private {
public:
    Private() : view(0) {}
    KoView *view;
    QMap<QString, KoToolDocker *> toolDockerMap;
    QMap<QString, bool> toolDockerVisibilityMap;
    QMap<QString, KoToolDocker *> activeToolDockerMap;
    QMap<QString, bool> toolDockerRaisedMap;
    void loadDocker(const QString& _name, bool visible);
    void removeDockers();
};

void KoDockerManager::Private::loadDocker(const QString &name, bool visible)
{
    KoToolDocker *td = new KoToolDocker();
    td->setObjectName(name);
    toolDockerMap[name] = td;
    toolDockerVisibilityMap[name] = visible;
    toolDockerRaisedMap[name] = false;
    td->setVisible(false);
    td->setEnabled(false);
    td->toggleViewAction()->setVisible(false);
}

void KoDockerManager::Private::removeDockers()
{
    // First remove the previous active dockers from sight and docker menu
    QMapIterator<QString, KoToolDocker *> iter(activeToolDockerMap);
    while (iter.hasNext()) {
        iter.next();

        // Check if the dock is raised or not
        QList<QDockWidget*> tabedDocks = view->shell()->tabifiedDockWidgets(iter.value());
        bool isOnTop = true;
        int idx = view->children().indexOf(iter.value());
        foreach (QDockWidget* dock, tabedDocks) {
            if (view->shell()->children().indexOf(dock) > idx && dock->isVisible() && dock->isEnabled()) {
                isOnTop = false;
                break;
            }
        }
        toolDockerRaisedMap[iter.key()] = isOnTop;
        //kDebug() << iter.value() << " " << iter.value()->isVisible() << iter.key();
        iter.value()->toggleViewAction()->setVisible(false);
        toolDockerVisibilityMap[iter.key()] = iter.value()->isVisible();
        iter.value()->setVisible(false);
        iter.value()->setEnabled(false);
    }
    activeToolDockerMap.clear();
}

KoDockerManager::KoDockerManager(KoView *view)
    : QObject(view), d( new Private() )
{
    d->view = view;

    KConfigGroup cfg = KGlobal::config()->group("DockerManager");

    QStringList visibleList = cfg.readEntry("VisibleToolDockers", QStringList());

    QStringListIterator iter(visibleList);
    while (iter.hasNext()) {
        QString name = iter.next();
        //kDebug() << "name = " << name;
        d->loadDocker(name, true);
        //kDebug() << "visible = " << d->toolDockerVisibilityMap[name];
    }
    QStringList hiddenList = cfg.readEntry("HiddenToolDockers", QStringList());

    QStringListIterator j2(hiddenList);
    while (j2.hasNext()) {
        QString name = j2.next();
        d->loadDocker(name, false);
    }
}

KoDockerManager::~KoDockerManager()
{
    d->removeDockers();
    KConfigGroup cfg = KGlobal::config()->group("DockerManager");
    QStringList visibleList;
    QStringList hiddenList;
    QMapIterator<QString, KoToolDocker *> iter(d->toolDockerMap);
    while (iter.hasNext()) {
        iter.next();
        if (d->toolDockerVisibilityMap.value(iter.key())) {
            visibleList += iter.key();
        } else {
            hiddenList += iter.key();
        }
    }
    //kDebug() << "visibleList = " << visibleList;
    //kDebug() << "hiddenList = " << hiddenList;
    cfg.writeEntry("VisibleToolDockers", visibleList);
    cfg.writeEntry("HiddenToolDockers", hiddenList);
    cfg.sync();
    delete d;
}

void KoDockerManager::removeUnusedOptionWidgets()
{
    QMapIterator<QString, KoToolDocker *> iter(d->toolDockerMap);
    while (iter.hasNext()) {
        iter.next();
        if (! d->activeToolDockerMap.contains(iter.key())) {
            //kDebug(30004) << "removing" << iter.key() << ((void*) iter.value());
            iter.value()->setVisible(false);
            iter.value()->setEnabled(false);
            iter.value()->toggleViewAction()->setVisible(false);
        } else {
            iter.value()->setVisible(d->toolDockerVisibilityMap[iter.key()]);
            iter.value()->setEnabled(true);
            iter.value()->toggleViewAction()->setVisible(true);
        }
    }
}

void KoDockerManager::newOptionWidgets(const QMap<QString, QWidget *> &optionWidgetMap, QWidget *callingView)
{
    Q_UNUSED(callingView);
    d->removeDockers();

    // Now show new active dockers (maybe even create) and show in docker menu
    QMapIterator<QString, QWidget *> iter(optionWidgetMap);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value()->objectName().isEmpty()) {
            kError(30004) << "tooldocker widget have no name " << iter.key();
            Q_ASSERT(!(iter.value()->objectName().isEmpty()));
            continue; // skip this docker in release build when assert don't crash
        }

        KoToolDocker *td = d->toolDockerMap[iter.value()->objectName()];

        if (!td) {
            td = new KoToolDocker();
            QString name = iter.value()->objectName();
            td->setObjectName(name);
            d->toolDockerMap[name] = td;
            d->toolDockerVisibilityMap[name] =  true;
        }
        td->setEnabled(true);
        td->setWindowTitle(iter.key());
        td->newOptionWidget(iter.value());
        d->view->restoreDockWidget(td);
        //kDebug() << iter.value()->objectName() << " " << d->toolDockerVisibilityMap[iter.value()->objectName()];
        td->setVisible(d->toolDockerVisibilityMap[iter.value()->objectName()]);
        //kDebug() << td->isVisible();
        td->toggleViewAction()->setVisible(true);
        d->activeToolDockerMap[iter.value()->objectName()] = td;
        if (d->toolDockerRaisedMap.value(iter.value()->objectName())) {
            td->raise();
        }
    }
}

#include "KoDockerManager.moc"
