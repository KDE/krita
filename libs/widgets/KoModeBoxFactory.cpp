/*
 * Copyright (c) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2011 C. Boemann <cbo@boemann.dk>
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

#include "KoModeBoxFactory.h"
#include <klocalizedstring.h>
#include "KoModeBox_p.h"
#include "KoModeBoxDocker_p.h"

class Q_DECL_HIDDEN KoModeBoxFactory::Private
{
public:
    KoCanvasControllerWidget *canvasController;
    QString applicationName;
};


KoModeBoxFactory::KoModeBoxFactory(KoCanvasControllerWidget *canvasController, const QString &applicationName, const QString& /*title*/)
    : d( new Private())
{
    d->canvasController = canvasController;
    d->applicationName = applicationName;
}

KoModeBoxFactory::~KoModeBoxFactory() {
    delete d;
}

QString KoModeBoxFactory::id() const
{
    return QString("ModeBox");
}

KoDockFactoryBase::DockPosition KoModeBoxFactory::defaultDockPosition() const
{
    return KoDockFactoryBase::DockRight;
}

QDockWidget* KoModeBoxFactory::createDockWidget()
{
    KoModeBox *box = new KoModeBox(d->canvasController, d->applicationName);
    QDockWidget *docker = new KoModeBoxDocker(box);

    return docker;
}
