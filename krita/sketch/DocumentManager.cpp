/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "DocumentManager.h"
#include "KisSketchPart.h"
#include "ProgressProxy.h"
#include "Settings.h"
#include "RecentFileManager.h"

#include <KoColorSpaceRegistry.h>

#include <kis_doc2.h>
#include <kis_image.h>

class DocumentManager::Private
{
public:
    Private()
        : proxy(0)
        , document(0)
        , part(0)
        , settingsManager(0)
        , recentFileManager(0)
        , newDocWidth(0)
        , newDocHeight(0)
        , newDocResolution(0)
        , importingDocument(false)
    { }
    ProgressProxy* proxy;
    QPointer<KisDoc2> document;
    QPointer<KisSketchPart> part;
    Settings* settingsManager;
    RecentFileManager* recentFileManager;

    QString saveAsFilename;
    QString openDocumentFilename;
    int newDocWidth, newDocHeight; float newDocResolution;
    bool importingDocument;
};

DocumentManager *DocumentManager::sm_instance = 0;

KisDoc2* DocumentManager::document() const
{
    return d->document;
}

KisSketchPart* DocumentManager::part()
{
    if (!d->part)
        d->part = new KisSketchPart(this);
    return d->part;
}

ProgressProxy* DocumentManager::progressProxy() const
{
    return d->proxy;
}

Settings* DocumentManager::settingsManager() const
{
    return d->settingsManager;
}

void DocumentManager::setSettingsManager(Settings* newManager)
{
    d->settingsManager = newManager;
}

RecentFileManager* DocumentManager::recentFileManager() const
{
    return d->recentFileManager;
}

void DocumentManager::newDocument(int width, int height, float resolution)
{
    closeDocument();

    d->newDocWidth = width;
    d->newDocHeight = height;
    d->newDocResolution = resolution;
    QTimer::singleShot(1000, this, SLOT(delayedNewDocument()));
}

void DocumentManager::delayedNewDocument()
{
    d->document = new KisDoc2(part());
    d->document->setProgressProxy(d->proxy);
    d->document->setSaveInBatchMode(true);
    part()->setDocument(d->document);
    d->document->newImage("New Image", d->newDocWidth, d->newDocHeight, KoColorSpaceRegistry::instance()->rgb8());
    d->document->image()->setResolution(d->newDocResolution, d->newDocResolution);

    emit documentChanged();
}

void DocumentManager::openDocument(const QString& document, bool import)
{
    closeDocument();
    d->openDocumentFilename = document;
    d->importingDocument = import;
    QTimer::singleShot(1000, this, SLOT(delayedOpenDocument()));
}

void DocumentManager::delayedOpenDocument()
{
    d->document = new KisDoc2(part());
    d->document->setProgressProxy(d->proxy);
    d->document->setSaveInBatchMode(true);
    part()->setDocument(d->document);

    d->document->setModified(false);
    if (d->importingDocument)
        d->document->importDocument(QUrl::fromLocalFile(d->openDocumentFilename));
    else
        d->document->openUrl(QUrl::fromLocalFile(d->openDocumentFilename));
    d->recentFileManager->addRecent(d->openDocumentFilename);
    emit documentChanged();
}

void DocumentManager::closeDocument()
{
    if (d->document) {
        emit aboutToDeleteDocument();
        d->document->closeUrl(false);
        //d->document->deleteLater();
        d->document = 0;
    }
}

bool DocumentManager::save()
{
    if (d->document->save())
    {
        d->recentFileManager->addRecent(d->document->url().toLocalFile());
        emit documentSaved();
        return true;
    }
    return false;
}

void DocumentManager::saveAs(const QString &filename, const QString &mimetype)
{
    d->document->setOutputMimeType(mimetype.toAscii());
    d->saveAsFilename = filename;
    // Yes. This is a massive hack. Basically, we need to wait a little while, to ensure
    // the save call happens late enough for a variety of UI things to happen first.
    // A second seems like a long time, but well, we do have file system interaction here,
    // so for now, we can get away with it.
    QTimer::singleShot(1000, this, SLOT(delayedSaveAs()));
}

void DocumentManager::delayedSaveAs()
{
    d->document->saveAs(d->saveAsFilename);
    d->settingsManager->setCurrentFile(d->saveAsFilename);
    emit documentSaved();
}

void DocumentManager::reload()
{
    KUrl url = d->document->url();
    closeDocument();
    d->openDocumentFilename = url.toLocalFile();
    QTimer::singleShot(0, this, SLOT(delayedOpenDocument()));
}

DocumentManager* DocumentManager::instance()
{
    if (!sm_instance) {
        sm_instance = new DocumentManager(QCoreApplication::instance());
    }

    return sm_instance;
}

DocumentManager::DocumentManager(QObject* parent)
    : QObject(parent), d(new Private)
{
    d->proxy = new ProgressProxy(this);
    d->recentFileManager = new RecentFileManager(this);
}

DocumentManager::~DocumentManager()
{
    delete d;
}

