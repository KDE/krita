/* This file is part of the KDE project
 * Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
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

#include "KoCopyController.h"

#include <KoTool.h>
#include <KoCanvasBase.h>
#include <KoToolProxy.h>
#include <KoToolSelection.h>

#include <KDebug>
#include <QAction>

// KoCopyControllerPrivate
class KoCopyControllerPrivate
{
public:
    KoCopyControllerPrivate(KoCopyController *p, KoCanvasBase *c, QAction *a);

    // request to start the actual copy
    void copy();

    // request to start the actual cut
    void cut();

    void selectionChanged(bool hasSelection);

    KoCopyController *parent;
    KoCanvasBase *canvas;
    QAction *action;
    bool appHasSelection;
};

KoCopyControllerPrivate::KoCopyControllerPrivate(KoCopyController *p, KoCanvasBase *c, QAction *a)
    : parent(p),
    canvas(c),
    action(a)
{
    appHasSelection = false;
}

void KoCopyControllerPrivate::copy()
{
    if (canvas->toolProxy()->selection() && canvas->toolProxy()->selection()->hasSelection())
        // means the copy can be done by a flake tool
        canvas->toolProxy()->copy();
    else // if not; then the application gets a request to do the copy
        emit parent->copyRequested();
}

void KoCopyControllerPrivate::cut()
{
    canvas->toolProxy()->cut();
}

void KoCopyControllerPrivate::selectionChanged(bool hasSelection)
{
    action->setEnabled(appHasSelection || hasSelection);
}


// KoCopyController
KoCopyController::KoCopyController(KoCanvasBase *canvas, QAction *copyAction)
    : QObject(copyAction),
    d(new KoCopyControllerPrivate(this, canvas, copyAction))
{
    connect(canvas->toolProxy(), SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    hasSelection(false);
}

KoCopyController::~KoCopyController()
{
    delete d;
}

void KoCopyController::hasSelection(bool selection)
{
    d->appHasSelection = selection;
    d->action->setEnabled(d->appHasSelection ||
            (d->canvas->toolProxy()->selection() && d->canvas->toolProxy()->selection()->hasSelection()));
}

#include <KoCopyController.moc>
