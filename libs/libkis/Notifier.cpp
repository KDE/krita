/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

    connect(KisPart::instance(), SIGNAL(sigMainWindowIsBeingCreated(KisMainWindow*)), SLOT(windowIsBeingCreated(KisMainWindow*)));

    connect(KisPart::instance(), SIGNAL(sigMainWindowCreated()), SIGNAL(windowCreated()));

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
    Document *doc = new Document(document, false);
    emit imageCreated(doc);
    delete doc;
}

void Notifier::viewCreated(KisView *view)
{
    View *v = new View(view);
    emit viewCreated(v);
    delete v;
}

void Notifier::viewClosed(KisView *view)
{
    View *v = new View(view);
    emit viewClosed(v);
    delete v;
}

void Notifier::windowIsBeingCreated(KisMainWindow *window)
{
    Window *w = new Window(window);
    emit windowIsBeingCreated(w);
    delete w;
}

