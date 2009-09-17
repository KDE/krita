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

#include "KoToolDockerFactory.h"
#include "KoToolDocker.h"

#include <QAction>
#include "KoView.h"
#include "KoMainWindow.h"

class KoDockerManager::Private {
public:
    Private() : view(0) {}
    KoView *view;
    QMap<QString, KoToolDocker *> toolDockerMap;
    QMap<QString, bool> toolDockerVisibillityMap;
    QMap<QString, KoToolDocker *> activeToolDockerMap;
    QMap<QString, bool> toolDockerRaisedMap;
    void loadDocker(const QString& _name, bool _visible);
    void removeDockers();
};

void KoDockerManager::Private::loadDocker(const QString& _name, bool _visible)
{
  KoToolDockerFactory toolDockerFactory(_name);
  KoToolDocker *td = qobject_cast<KoToolDocker *>(view->createDockWidget(&toolDockerFactory));
  toolDockerMap[_name] = td;
  toolDockerVisibillityMap[_name] = _visible;
  toolDockerRaisedMap[_name] = false;
  td->setVisible(false);
  td->setEnabled(false);
  td->toggleViewAction()->setVisible(false);
}

void KoDockerManager::Private::removeDockers()
{
    // First remove the previous active dockers from sight and docker menu
    QMapIterator<QString, KoToolDocker *> j(activeToolDockerMap);
    while (j.hasNext()) {
        j.next();

        // Check if the dock is raised or not
        QList<QDockWidget*> tabedDocks = view->shell()->tabifiedDockWidgets(j.value());
        bool isOnTop = true;
        int idx = view->children().indexOf(j.value());
        foreach(QDockWidget* dock, tabedDocks) {
          if(view->shell()->children().indexOf(dock) > idx and dock->isVisible() and dock->isEnabled()) {
            isOnTop = false;
            break;
          }
        }
        toolDockerRaisedMap[j.key()] = isOnTop;
        //kDebug() << j.value() << " " << j.value()->isVisible() << j.key();
        j.value()->toggleViewAction()->setVisible(false);
        toolDockerVisibillityMap[j.key()] = j.value()->isVisible();
        j.value()->setVisible(false);
        j.value()->setEnabled(false);
    }
    activeToolDockerMap.clear();
}

KoDockerManager::KoDockerManager(KoView *view)
    : QObject(view), d( new Private() )
{
    d->view = view;

    KConfigGroup cfg = KGlobal::config()->group("DockerManager");

    QStringList visibleList = cfg.readEntry("VisibleToolDockers", QStringList());

    QStringListIterator j(visibleList);
    while (j.hasNext()) {
      QString name = j.next();
      //kDebug() << "name = " << name;
      d->loadDocker(name, true);
      //kDebug() << "visible = " << d->toolDockerVisibillityMap[name];
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
    QMapIterator<QString, KoToolDocker *> j(d->toolDockerMap);
    while (j.hasNext()) {
        j.next();
        if(d->toolDockerVisibillityMap[j.key()]) {
          visibleList += j.key();
        } else {
          hiddenList += j.key();
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
  QMapIterator<QString, KoToolDocker *> j(d->toolDockerMap);
  while (j.hasNext()) {
    j.next();
    if (not d->activeToolDockerMap.contains(j.key())) {
        //kDebug(30004) << "removing" << j.key() << ((void*) j.value());
        j.value()->setVisible(false);
        j.value()->setEnabled(false);
        j.value()->toggleViewAction()->setVisible(false);
    } else {
        j.value()->setVisible(d->toolDockerVisibillityMap[j.key()]);
        j.value()->setEnabled(true);
        j.value()->toggleViewAction()->setVisible(true);
    }
  }
}

void KoDockerManager::newOptionWidgets(const QMap<QString, QWidget *> & optionWidgetMap, QWidget *callingView)
{
    Q_UNUSED(callingView);
    d->removeDockers();

    // Now show new active dockers (maybe even create) and show in docker menu
    QMapIterator<QString, QWidget *> i(optionWidgetMap);
    while (i.hasNext()) {
        i.next();
        if (i.value()->objectName().isEmpty()) {
            kError(30004) << "tooldocker widget have no name " << i.key() << " " << i.value()->objectName();
            Q_ASSERT(!(i.value()->objectName().isEmpty()));
            continue; // skip this docker in release build when assert don't crash
        }

        KoToolDocker *td = d->toolDockerMap[i.value()->objectName()];

        if(!td) {
            KoToolDockerFactory toolDockerFactory(i.value()->objectName());
            td = qobject_cast<KoToolDocker*>(d->view->createDockWidget(&toolDockerFactory));
            if (!td)
                return;
            d->toolDockerMap[i.value()->objectName()] = td;
            d->toolDockerVisibillityMap[i.value()->objectName()] =  true;
        }
        td->setEnabled(true);
        td->setWindowTitle(i.key());
        td->newOptionWidget(i.value());
        d->view->restoreDockWidget(td);
        //kDebug() << i.value()->objectName() << " " << d->toolDockerVisibillityMap[i.value()->objectName()];
        td->setVisible(d->toolDockerVisibillityMap[i.value()->objectName()]);
        //kDebug() << td->isVisible();
        td->toggleViewAction()->setVisible(true);
        d->activeToolDockerMap[i.value()->objectName()] = td;
        if(d->toolDockerRaisedMap[i.value()->objectName()]) {
          td->raise();
        }
    }
}

#include "KoDockerManager.moc"
