/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "View.h"
#include <QPointer>

#include <KisView.h>
#include <KisViewManager.h>
#include <kis_selection_manager.h>
#include <KisMainWindow.h>
#include <KoCanvasBase.h>
#include <kis_canvas2.h>
#include "Document.h"
#include "Canvas.h"
#include "Window.h"

struct View::Private {
    Private() {}
    QPointer<KisView> view;
};

View::View(KisView* view, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->view = view;
}

View::~View()
{
    delete d;
}

Window* View::window() const
{
    if (!d->view) return 0;
    KisMainWindow *mainwin = d->view->mainWindow();
    Window *win = new Window(mainwin);
    return win;
}


Document* View::document() const
{
    if (!d->view) return 0;
    Document *doc = new Document(d->view->document());
    return doc;
}

bool View::visible() const
{
    if (!d->view) return false;
    return d->view->isVisible();
}

void View::setVisible()
{
    if (!d->view) return;
    KisMainWindow *mainwin = d->view->mainWindow();
    mainwin->setActiveView(d->view);
    mainwin->subWindowActivated();
}

Canvas* View::canvas() const
{
    if (!d->view) return 0;
    Canvas *c = new Canvas(d->view->canvasBase());
    return c;
}

void View::close(bool confirm)
{
    if (!d->view) return;
}

KisView *View::view()
{
    return d->view;
}



