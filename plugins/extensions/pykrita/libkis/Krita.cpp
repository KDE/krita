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

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoDockRegistry.h>
#include <kactioncollection.h>
#include <KisPart.h>
#include <KisMainWindow.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_action.h>
#include <kis_script_manager.h>
#include <KisViewManager.h>
#include <KritaVersionWrapper.h>

#include "View.h"
#include "Document.h"
#include "Window.h"
#include "ViewExtension.h"
#include "DockWidgetFactoryBase.h"

#include <QVariant>

Krita* Krita::s_instance = 0;

struct Krita::Private {
    Private() {}
    QList<ViewExtension*> viewExtensions;
    bool batchMode {false};
    Notifier *notifier{new Notifier()};
};

Krita::Krita(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    qRegisterMetaType<Notifier*>();
}

Krita::~Krita()
{
    qDeleteAll(d->viewExtensions);
    delete d->notifier;
    delete d;
}

QList<Action *> Krita::actions() const
{
    QList<Action*> actionList;
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return actionList;
    }
    KActionCollection *actionCollection = mainWindow->actionCollection();
    Q_FOREACH(QAction *action, actionCollection->actions()) {
        actionList << new Action(action->objectName(), action);
    }
    return actionList;
}

Action *Krita::action(const QString &name) const
{
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return 0;
    }
    KActionCollection *actionCollection = mainWindow->actionCollection();
    QAction *action = actionCollection->action(name);
    if (action) {
        return new Action(name, action);
    }
    return 0;
}

Document* Krita::activeDocument() const
{
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return 0;
    }
    KisView *view = mainWindow->activeView();
    if (!view) {
        return 0;
    }
    KisDocument *document = view->document();
    return new Document(document);
}

void Krita::setActiveDocument(Document* value)
{
    Q_FOREACH(KisView *view, KisPart::instance()->views()) {
        if (view->document() == value->document().data()) {
            view->activateWindow();
            break;
        }
    }
}

bool Krita::batchmode() const
{
    return d->batchMode;
}

void Krita::setBatchmode(bool value)
{
    d->batchMode = value;
}


QList<Document *> Krita::documents() const
{
    QList<Document *> ret;
    foreach(QPointer<KisDocument> doc, KisPart::instance()->documents()) {
        ret << new Document(doc);
    }
    return ret;
}

QList<Filter*> Krita::filters() const
{
    return QList<Filter*>();
}

QList<Generator*> Krita::generators() const
{
    return QList<Generator*> ();
}

Notifier* Krita::notifier() const
{
    return d->notifier;
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
    return KritaVersionWrapper::versionString(true);
}

QList<View *> Krita::views() const
{
    QList<View *> ret;
    foreach(QPointer<KisView> view, KisPart::instance()->views()) {
        ret << new View(view);
    }
    return ret;
}

Window *Krita::activeWindow() const
{
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (!mainWindow) {
        return 0;
    }
    return new Window(mainWindow);
}

QList<Window*>  Krita::windows() const
{
    QList<Window*> ret;
    foreach(QPointer<KisMainWindow> mainWin, KisPart::instance()->mainWindows()) {
        ret << new Window(mainWin);
    }
    return ret;

}

QList<Resource *> Krita::resources() const
{
    return QList<Resource *> ();
}

void Krita::setResources(QList<Resource *> value)
{

}

void Krita::addDockWidget(DockWidget *dockWidget)
{
}

void Krita::addAction(Action *action)
{
}

bool Krita::closeApplication()
{
    qDebug() << "closeApplication called";
    return false;
}

Document* Krita::createDocument(int width, int height, const QString &name, const QString &colorModel, const QString &colorDepth, const QString &profile)
{
    KisDocument *document = KisPart::instance()->createDocument();
    KisPart::instance()->addDocument(document);
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, profile);
    QColor qc(Qt::white);
    qc.setAlpha(0);
    KoColor bgColor(qc, cs);

    document->newImage(name, width, height, cs, bgColor, true, 1, "", 100.0);

    return new Document(document, true);
}

Document* Krita::openDocument(const QString &filename)
{
    KisDocument *document = KisPart::instance()->createDocument();
    KisPart::instance()->addDocument(document);
    document->openUrl(QUrl::fromLocalFile(filename), KisDocument::OPEN_URL_FLAG_DO_NOT_ADD_TO_RECENT_FILES);
    return new Document(document);
}

Window* Krita::openWindow()
{
    return 0;
}


QAction *Krita::createAction(const QString &text)
{
    KisAction *action = new KisAction(text, this);
    KisPart::instance()->addScriptAction(action);
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

/**
 * Scripter.fromVariant(variant)
 * variant is a QVariant
 * returns instance of QObject-subclass
 *
 * This is a helper method for PyQt
 * Because PyQt cannot cast a variant to a QObject or QWidget
 * I hope that will change some time.
 */
QObject *Krita::fromVariant(const QVariant& v)
{

    if (v.canConvert< QWidget* >())
    {
        QObject* obj = qvariant_cast< QWidget* >(v);
        return obj;
    }
    else if (v.canConvert< QObject* >())
    {
        QObject* obj = qvariant_cast< QObject* >(v);
        return obj;
    }
    else
        return 0;
}

