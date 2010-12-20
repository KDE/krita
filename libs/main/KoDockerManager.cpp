/* This file is part of the KDE project
 *
 * Copyright (c) 2008,2010 Casper Boemann <cbo@boemann.dk>
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
#include "KoDockFactoryBase.h"

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

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


class KoDockerManager::Private
{
public:
    Private() {};
    KoToolDocker *docker;

    void makeDockVisible()
    {
    }
};

KoDockerManager::KoDockerManager(KoMainWindow *mainWindow)
    : QObject(mainWindow), d( new Private() )
{
    ToolDockerFactory factory;
    d->docker = qobject_cast<KoToolDocker*>(mainWindow->createDockWidget(&factory));
    Q_ASSERT(d->docker);
    connect(mainWindow, SIGNAL(restoringDone()), this, SLOT(makeDockVisible()));
}

KoDockerManager::~KoDockerManager()
{
    delete d;
}

void KoDockerManager::newOptionWidgets(const QMap<QString, QWidget *> &optionWidgetMap)
{
    d->docker->setOptionWidgets(optionWidgetMap);
}

#include <KoDockerManager.moc>
