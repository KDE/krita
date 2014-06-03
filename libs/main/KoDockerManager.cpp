/* This file is part of the KDE project
 *
 * Copyright (c) 2008-2012 C. Boemann <cbo@boemann.dk>
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
#include "KoDockerManager_p.h"
#include "KoDockFactoryBase.h"

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <ktoolbar.h>

#include "KoToolDocker_p.h"

#include "KoView.h"
#include "KoMainWindow.h"

class ToolDockerFactory : public KoDockFactoryBase
{
public:
    ToolDockerFactory() : KoDockFactoryBase() { }

    QString id() const {
        return "sharedtooldocker";
    }

    QDockWidget* createDockWidget() {
        KoToolDocker * dockWidget = new KoToolDocker();
        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockRight;
    }
};

KoDockerManager::KoDockerManager(KoMainWindow *mainWindow)
    : QObject(mainWindow), d( new Private(mainWindow) )
{
    ToolDockerFactory toolDockerFactory;
    d->toolOptionsDocker =
            qobject_cast<KoToolDocker*>(mainWindow->createDockWidget(&toolDockerFactory));
    Q_ASSERT(d->toolOptionsDocker);
    d->toolOptionsDocker->setVisible(false);

    connect(mainWindow, SIGNAL(restoringDone()), this, SLOT(restoringDone()));
}

KoDockerManager::~KoDockerManager()
{
    delete d;
}

void KoDockerManager::newOptionWidgets(const QList<QWidget *> &optionWidgetList)
{
    d->toolOptionsDocker->setOptionWidgets(optionWidgetList);
}

void KoDockerManager::removeToolOptionsDocker()
{
    d->toolOptionsDocker->setVisible(false);
    d->showOptionsDocker = false;
}

void KoDockerManager::setIcons(bool enabled)
{
    d->toolOptionsDocker->setTabEnabled(enabled);
}

void KoDockerManager::resetToolDockerWidgets()
{
    d->toolOptionsDocker->resetWidgets();
}

#include <KoDockerManager.moc>
