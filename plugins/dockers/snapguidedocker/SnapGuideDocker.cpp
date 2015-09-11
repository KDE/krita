/* This file is part of the KDE project
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002 Rob Buis <buis@kde.org>
   Copyright (C) 2004 Laurent Montel <montel@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 C. Boemann <cbo@boemann.dk>
   Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "SnapGuideDocker.h"

#include <klocalizedstring.h>

#include <KoCanvasBase.h>
#include <KoCanvasObserverBase.h>
#include <KoSnapGuide.h>

class SnapGuideDocker::Private
{
public:
    Private()
        : canvas(0)
        , mainWidget(0)
    {}

    KoCanvasBase *canvas;
    QWidget *mainWidget;
};


SnapGuideDocker::SnapGuideDocker()
    : d(new Private())
{
    setWindowTitle(i18n("Snap Settings"));
}

SnapGuideDocker::~SnapGuideDocker()
{
    delete d;
}


void SnapGuideDocker::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);

    if (d->canvas) {
        d->canvas->disconnectCanvasObserver(this); // "Every connection you make emits a signal, so duplicate connections emit two signals"
    }

    if (canvas) {
        d->mainWidget = canvas->createSnapGuideConfigWidget();
    }

    d->canvas = canvas;
    setWidget(d->mainWidget);
}

void SnapGuideDocker::unsetCanvas()
{
    setEnabled(false);
    setWidget(0);
    d->canvas = 0;
}

void SnapGuideDocker::locationChanged(Qt::DockWidgetArea area)
{
    Q_UNUSED(area);
}
