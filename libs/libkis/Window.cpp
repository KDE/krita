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
#include "Window.h"

#include <QMenuBar>
#include <QObject>
#include <QAction>

#include <kis_action.h>
#include <KisMainWindow.h>
#include <KisPart.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <kis_action_manager.h>
#include <kis_debug.h>

#include <Document.h>
#include <View.h>


struct Window::Private {
    Private() {}

    QPointer<KisMainWindow> window;
};

Window::Window(KisMainWindow *window, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->window = window;
    connect(window, SIGNAL(destroyed(QObject*)), SIGNAL(windowClosed()));
    connect(window, SIGNAL(themeChanged()), SIGNAL(themeChanged()));
    connect(window, SIGNAL(activeViewChanged()), SIGNAL(activeViewChanged()));
}

Window::~Window()
{
    delete d;
}

bool Window::operator==(const Window &other) const
{
    return (d->window == other.d->window);
}

bool Window::operator!=(const Window &other) const
{
    return !(operator==(other));
}

QMainWindow *Window::qwindow() const
{
    return d->window;
}

QList<View*> Window::views() const
{
    QList<View *> ret;
    if (d->window) {
        foreach(QPointer<KisView> view, KisPart::instance()->views()) {
            if (view->mainWindow() == d->window) {
                ret << new View(view);
            }
        }
    }
    return ret;

}

View *Window::addView(Document *document)
{
    if (d->window) {
        // Once the document is shown in the ui, it's owned by Krita
        // If the Document instance goes out of scope, it shouldn't
        // delete the owned image.
        document->setOwnsDocument(false);
        KisView *view = d->window->newView(document->document());
        return new View(view);
    }
    return 0;
}

void Window::showView(View *view)
{
    if (views().contains(view)) {
        KisView *v = view->view();
        d->window->showView(v);
    }
}

View *Window::activeView() const
{
    if (d->window) {
        return new View(d->window->activeView());
    }
    return 0;
}

void Window::activate()
{
    if (d->window) {
        d->window->activateWindow();
    }
}

void Window::close()
{
    if (d->window) {
        KisPart::instance()->removeMainWindow(d->window);
        d->window->close();
    }
}


QAction *Window::createAction(const QString &id, const QString &text, const QString &menuLocation)
{
    KisAction *action = d->window->viewManager()->actionManager()->createAction(id);
    if (!text.isEmpty()) {
        action->setText(text);
    }
    action->setObjectName(id);
    action->setProperty("menulocation", menuLocation);
    return action;
}



