/*
 * Copyright (c) 2006 Peter Simonsson <peter.simonsson@gmail.com>
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

#include "KoToolBoxFactory.h"
#include <klocale.h>
#include <KoToolManager.h>
#include "KoToolBox_p.h"

class KoToolBoxFactory::Private {
public:
    KoCanvasController *canvas;
    QString appName;
};


KoToolBoxFactory::KoToolBoxFactory(KoCanvasController *canvas, const QString& title)
    : d( new Private())
{
    if (title.isEmpty()) {
        d->appName = i18n("Tools");
    }
    else {
        d->appName = title;
    }
    d->canvas = canvas;
}

KoToolBoxFactory::~KoToolBoxFactory() {
    delete d;
}

QString KoToolBoxFactory::id() const
{
    return QString("ToolBox");
}

KoDockFactoryBase::DockPosition KoToolBoxFactory::defaultDockPosition() const
{
    return KoDockFactoryBase::DockLeft;
}

QDockWidget* KoToolBoxFactory::createDockWidget()
{
    KoToolBox *box = new KoToolBox(d->canvas);
    KoToolBoxDocker *docker = new KoToolBoxDocker(box);
    docker->setWindowTitle(d->appName);
    docker->setObjectName("ToolBox_"+ d->appName);

    return docker;
}
