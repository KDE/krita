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
#include "Notifier.h"
#include <KisApplication.h>
#include <KisPart.h>
#include <kis_config_notifier.h>
#include "View.h"
#include "Window.h"
#include "Document.h"

struct Notifier::Private {
    Private() {}
    bool active {false};
};

Notifier::Notifier(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    connect(qApp, SIGNAL(aboutToQuit()), SIGNAL(applicationClosing()));

    connect(KisPart::instance(), SIGNAL(sigDocumentAdded(KisDocument*)), SLOT(imageCreated(KisDocument*)));
    connect(KisPart::instance(), SIGNAL(sigDocumentSaved(QString)), SIGNAL(imageSaved(QString)));
    connect(KisPart::instance(), SIGNAL(sigDocumentRemoved(QString)), SIGNAL(imageClosed(QString)));

    connect(KisPart::instance(), SIGNAL(sigViewAdded(KisView*)), SLOT(viewCreated(KisView*)));
    connect(KisPart::instance(), SIGNAL(sigViewRemoved(KisView*)), SLOT(viewClosed(KisView*)));

    connect(KisPart::instance(), SIGNAL(sigWindowAdded(KisMainWindow*)), SLOT(windowCreated(KisMainWindow*)));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SIGNAL(configurationChanged()));

}


Notifier::~Notifier()
{
    delete d;
}

bool Notifier::active() const
{
    return d->active;
}

void Notifier::setActive(bool value)
{
    d->active = value;
    blockSignals(!value);
}

void Notifier::imageCreated(KisDocument* document)
{
    Document *doc = new Document(document);
    emit imageCreated(doc);
}

void Notifier::viewCreated(KisView *view)
{
    View *v = new View(view);
    emit viewCreated(v);
}

void Notifier::viewClosed(KisView *view)
{
    View *v = new View(view);
    emit viewClosed(v);
}

void Notifier::windowCreated(KisMainWindow *window)
{
    Window *w = new Window(window);
    emit windowCreated(w);
}

