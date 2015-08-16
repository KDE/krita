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

QDockWidget* KoToolBoxFactory::createDockWidget()
{
    KoToolBox *box = new KoToolBox();
    KoToolBoxDocker *docker = new KoToolBoxDocker(box);
    docker->setObjectName(id());

    return docker;
}
