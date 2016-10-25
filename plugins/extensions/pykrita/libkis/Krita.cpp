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
#include "Krita.h"

#include <QPointer>

#include <KoDockRegistry.h>

#include <KisPart.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_action.h>
#include <kis_script_manager.h>
#include <KisViewManager.h>

#include "View.h"
#include "Document.h"
#include "Window.h"
#include "ViewExtension.h"
#include "DockWidgetFactoryBase.h"

Krita* Krita::s_instance = 0;

struct Krita::Private {
    Private() {}
    QList<ViewExtension*> viewExtensions;
};

Krita::Krita(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Krita::~Krita()
{
    qDeleteAll(d->viewExtensions);
    delete d;
}

QList<Action*> Krita::actions() const
{
    return QList<Action*>();
}

Document* Krita::activeDocument() const
{
    return 0;
}

void Krita::setActiveDocument(Document* value)
{
}


bool Krita::batchmode() const
{
    return false;
}

void Krita::setBatchmode(bool value)
{
}


QList<Document*> Krita::documents() const
{
    QList<Document *> ret;
    foreach(QPointer<KisDocument> doc, KisPart::instance()->documents()) {
        ret << new Document(doc);
    }
    return ret;
}

QList<Exporter*> Krita::exporters() const
{
    return QList<Exporter*>();
}

QList<Filter*> Krita::filters() const
{
    return QList<Filter*>();
}

QList<Generator*> Krita::generators() const
{
    return QList<Generator*>();
}

QList<Importer*> Krita::importers() const
{
    return QList<Importer*>();
}

Notifier* Krita::notifier() const
{
    return 0;
}


InfoObject* Krita::preferences() const
{
    return 0;
}

void Krita::setPreferences(InfoObject* value)
{
}


QString Krita::version() const
{
    return QString();
}

QList<View*> Krita::views() const
{
    QList<View *> ret;
    foreach(QPointer<KisView> view, KisPart::instance()->views()) {
        ret << new View(view);
    }
    return ret;
}

QList<Window*> Krita::windows() const
{
    QList<Window *> ret;
    foreach(QPointer<KisMainWindow> mainWin, KisPart::instance()->mainWindows()) {
        ret << new Window(mainWin);
    }
    return ret;

}

QList<Resource*> Krita::resources() const
{
    return QList<Resource*>();
}

void Krita::addDockWidget(DockWidget *dockWidget)
{
}

void Krita::addAction(Action *action)
{
}

bool Krita::closeApplication()
{
    return false;
}

Document* Krita::createDocument()
{
    return 0;
}

Document* Krita::openDocument()
{
    return 0;
}

Window* Krita::openWindow()
{
    return 0;
}


QAction *Krita::createAction(const QString &text)
{
    KisAction *action = new KisAction(text, this);
    foreach(KisMainWindow *mainWin, KisPart::instance()->mainWindows()) {
        mainWin->viewManager()->scriptManager()->addAction(action);
    }
    return action;
}

void Krita::addViewExtension(ViewExtension* viewExtension)
{
    d->viewExtensions.append(viewExtension);
}

QList< ViewExtension* > Krita::viewExtensions()
{
    return d->viewExtensions;
}

void Krita::addDockWidgetFactory(DockWidgetFactoryBase* factory)
{
    KoDockRegistry::instance()->add(factory);
}

Krita* Krita::instance()
{
    if (!s_instance)
    {
        s_instance = new Krita;
    }
    return s_instance;
}

