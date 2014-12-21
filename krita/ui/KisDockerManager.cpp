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
#include "KisDockerManager.h"
#include "KisDockerManager_p.h"
#include "KoDockFactoryBase.h"

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <kglobalsettings.h>

#include "KoToolDocker.h"

#include "KisView.h"
#include "KisMainWindow.h"

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

KisDockerManager::KisDockerManager(KisMainWindow *mainWindow)
    : QObject(mainWindow), d( new Private(mainWindow) )
{
    ToolDockerFactory toolDockerFactory;
    d->toolOptionsDocker =
            qobject_cast<KoToolDocker*>(mainWindow->createDockWidget(&toolDockerFactory));
    Q_ASSERT(d->toolOptionsDocker);
    d->toolOptionsDocker->setVisible(false);

    connect(mainWindow, SIGNAL(restoringDone()), this, SLOT(restoringDone()));
}

KisDockerManager::~KisDockerManager()
{
    delete d;
}

void KisDockerManager::newOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList)
{
    d->toolOptionsDocker->setOptionWidgets(optionWidgetList);

    KConfigGroup group(KGlobal::config(), "GUI");
    QFont dockWidgetFont  = KGlobalSettings::generalFont();
    qreal pointSize = group.readEntry("palettefontsize", dockWidgetFont.pointSize() * 0.75);
    pointSize = qMax(pointSize, KGlobalSettings::smallestReadableFont().pointSizeF());
    dockWidgetFont.setPointSizeF(pointSize);

    foreach(QWidget *w, optionWidgetList) {
#ifdef Q_OS_MAC
        w->setAttribute(Qt::WA_MacSmallSize, true);
#endif
        w->setFont(dockWidgetFont);
    }



}

void KisDockerManager::removeToolOptionsDocker()
{
    d->toolOptionsDocker->setVisible(false);
    d->showOptionsDocker = false;
}

void KisDockerManager::setIcons(bool enabled)
{
    d->toolOptionsDocker->setTabEnabled(enabled);
}

void KisDockerManager::resetToolDockerWidgets()
{
    d->toolOptionsDocker->resetWidgets();
}

#include <KisDockerManager.moc>
