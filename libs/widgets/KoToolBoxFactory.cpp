/*
 * SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoToolBoxFactory.h"
#include "KoToolBox_p.h"
#include "KoToolBoxDocker_p.h"


KoToolBoxFactory::KoToolBoxFactory()
{
}

KoToolBoxFactory::~KoToolBoxFactory() {
}

QString KoToolBoxFactory::id() const
{
    return QLatin1String("ToolBox");
}

KoDockFactoryBase::DockPosition KoToolBoxFactory::defaultDockPosition() const
{
    return KoDockFactoryBase::DockLeft;
}

KDDockWidgets::DockWidgetBase* KoToolBoxFactory::createDockWidget()
{
    KoToolBox *box = new KoToolBox();
    KoToolBoxDocker *docker = new KoToolBoxDocker(box);
    docker->setObjectName(id());

    return docker;
}
