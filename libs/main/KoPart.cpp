/* This file is part of the KDE project
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2000-2005 David Faure <faure@kde.org>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010-2012 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011 Inge Wallin <ingwa@kogmbh.com>
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

#include "KoApplication.h"
#include "KoPart.h"
#include "KoMainWindow.h"
#include "KoDocument.h"
#include "KoView.h"
#include "KoCanvasController.h"
#include "KoCanvasControllerWidget.h"
#include "KoOpenPane.h"
#include "KoMainWindow.h"
#include "KoProgressProxy.h"
#include "KoFilterManager.h"
#include "KoServiceProvider.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kxmlguifactory.h>
#include <kdeprintdialog.h>
#include <knotification.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kfileitem.h>
#include <kio/netaccess.h>

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

namespace {

class DocumentProgressProxy : public KoProgressProxy {
public:
    KoMainWindow *m_shell;
    DocumentProgressProxy(KoMainWindow *shell)
        : m_shell(shell)
    {
    }

    ~DocumentProgressProxy() {
        // signal that the job is done
        setValue(-1);
    }

    int maximum() const {
        return 100;
    }

    void setValue(int value) {
        if (m_shell) {
            m_shell->slotProgress(value);
        }
    }

    void setRange(int /*minimum*/, int /*maximum*/) {

    }

    void setFormat(const QString &/*format*/) {

    }
};
}


class KoPart::Private
{
public:
    Private()
        : document(0)
        , canvasItem(0)
        , startUpWidget(0)
    {
    }

    QList<KoView*> views;
    QList<KoMainWindow*> shells;
    KoDocument *document;
    QGraphicsItem *canvasItem;
    QPointer<KoOpenPane> startUpWidget;
    QString templateType;

};


KoPart::KoPart(QObject *parent)
        : KParts::ReadWritePart(parent)
        , d(new Private)
{
    // we're not a part in a part, so we cannot be selected, we're always top-level
    setSelectable(false);


    connect(this, SIGNAL(started(KIO::Job*)), SLOT(slotStarted(KIO::Job*)));
}

KoPart::~KoPart()
{

    // Tell our views that the document is already destroyed and
    // that they shouldn't try to access it.
    foreach(KoView *view, views()) {
        view->setDocumentDeleted();
    }

    while (!d->shells.isEmpty()) {
        delete d->shells.takeFirst();
    }


    delete d->startUpWidget;
    d->startUpWidget = 0;


    delete d;
}

void KoPart::setDocument(KoDocument *document)
{
    Q_ASSERT(document);
    d->document = document;
    connect(d->document, SIGNAL(titleModified(QString,bool)), SLOT(setTitleModified(QString,bool)));
}

KoDocument *KoPart::document() const
{
    Q_ASSERT(d->document);
    return d->document;
}

void KoPart::setReadWrite(bool readwrite)
{
    KParts::ReadWritePart::setReadWrite(readwrite);

    foreach(KoView *view, d->views) {
        view->updateReadWrite(readwrite);
    }

    foreach(KoMainWindow *mainWindow, d->shells) {
        mainWindow->setReadWrite(readwrite);
    }
}

bool KoPart::openFile()
{
    KoMainWindow *shell = 0;
    if (shellCount() > 0) {
        shell = shells()[0];
    }
    DocumentProgressProxy progressProxy(shell);
    d->document->setProgressProxy(&progressProxy);
    d->document->setUrl(url());

    // THIS IS WRONG! KoDocument::openFile should move here, and whoever subclassed KoDocument to
    // reimplement openFile shold now subclass KoPart.
    return d->document->openFile();
}

bool KoPart::saveFile()
{
    KoMainWindow *shell = 0;
    if (shellCount() > 0) {
        shell = shells()[0];
    }
    DocumentProgressProxy progressProxy(shell);
    d->document->setProgressProxy(&progressProxy);
    d->document->setUrl(url());

    // THIS IS WRONG! KoDocument::saveFile should move here, and whoever subclassed KoDocument to
    // reimplement saveFile shold now subclass KoPart.
    return d->document->saveFile();
}

KoView *KoPart::createView(QWidget *parent)
{
    KoView *view = createViewInstance(parent);
    addView(view);
    return view;
}

void KoPart::addView(KoView *view)
{
    if (!view)
        return;

    d->views.append(view);
    view->updateReadWrite(isReadWrite());

    if (d->views.size() == 1) {
        KoApplication *app = qobject_cast<KoApplication*>(KApplication::kApplication());
        if (0 != app) {
            emit app->documentOpened('/'+objectName());
        }
    }
}

void KoPart::removeView(KoView *view)
{
    d->views.removeAll(view);

    if (d->views.isEmpty()) {
        KoApplication *app = qobject_cast<KoApplication*>(KApplication::kApplication());
        if (0 != app) {
            emit app->documentClosed('/'+objectName());
        }
    }
}

QList<KoView*> KoPart::views() const
{
    return d->views;
}

int KoPart::viewCount() const
{
    return d->views.count();
}

QGraphicsItem *KoPart::canvasItem(bool create)
{
    if (create && !d->canvasItem) {
        d->canvasItem = createCanvasItem();
    }
    return d->canvasItem;
}

QGraphicsItem *KoPart::createCanvasItem()
{
    KoView *view = createView();
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    QWidget *canvasController = view->findChild<KoCanvasControllerWidget*>();
    proxy->setWidget(canvasController);
    return proxy;
}

void KoPart::addShell(KoMainWindow *shell)
{
    if (d->shells.indexOf(shell) == -1) {
        //kDebug(30003) <<"shell" << (void*)shell <<"added to doc" << this;
        d->shells.append(shell);
        // XXX!!!
        //connect(shell, SIGNAL(documentSaved()), d->undoStack, SLOT(setClean()));
    }
}

void KoPart::removeShell(KoMainWindow *shell)
{
    //kDebug(30003) <<"shell" << (void*)shell <<"removed from doc" << this;
    if (shell) {
        disconnect(shell, SIGNAL(documentSaved()), d->document->undoStack(), SLOT(setClean()));
        d->shells.removeAll(shell);
    }
}

const QList<KoMainWindow*>& KoPart::shells() const
{
    return d->shells;
}

int KoPart::shellCount() const
{
    return d->shells.count();
}


KoMainWindow *KoPart::currentShell() const
{
    QWidget *widget = qApp->activeWindow();
    KoMainWindow *shell = qobject_cast<KoMainWindow*>(widget);
    while (!shell && widget) {
        widget = widget->parentWidget();
        shell = qobject_cast<KoMainWindow*>(widget);
    }

    if (!shell && d->document && shells().size() > 0) {
        shell = shells().first();
    }
    return shell;

}

void KoPart::showSavingErrorDialog()
{
    if (d->document->errorMessage().isEmpty()) {
        KMessageBox::error(0, i18n("Could not save\n%1", localFilePath()));
    } else if (d->document->errorMessage() != "USER_CANCELED") {
        KMessageBox::error(0, i18n("Could not save %1\nReason: %2", localFilePath(), d->document->errorMessage()));
    }
}

void KoPart::showLoadingErrorDialog()
{
    if (d->document->errorMessage().isEmpty()) {
        KMessageBox::error(0, i18n("Could not open\n%1", localFilePath()));
    } else if (d->document->errorMessage() != "USER_CANCELED") {
        KMessageBox::error(0, i18n("Could not open %1\nReason: %2", localFilePath(), d->document->errorMessage()));
    }
}

void KoPart::openExistingFile(const KUrl& url)
{
    qApp->setOverrideCursor(Qt::BusyCursor);
    d->document->openUrl(url);
    setModified(false);
    qApp->restoreOverrideCursor();
}

void KoPart::openTemplate(const KUrl& url)
{
    qApp->setOverrideCursor(Qt::BusyCursor);
    bool ok = d->document->loadNativeFormat(url.toLocalFile());
    d->document->setModified(false);
    d->document->undoStack()->clear();

    if (ok) {
        QString mimeType = KMimeType::findByUrl( url, 0, true )->name();
        // in case this is a open document template remove the -template from the end
        mimeType.remove( QRegExp( "-template$" ) );
        d->document->setMimeTypeAfterLoading(mimeType);
        deleteOpenPane();
        d->document->resetURL();
        d->document->setEmpty();
    } else {
        showLoadingErrorDialog();
        d->document->initEmpty();
    }
    qApp->restoreOverrideCursor();
}

void KoPart::addRecentURLToAllShells(KUrl url)
{
    // Add to recent actions list in our shells
    foreach(KoMainWindow *mainWindow, d->shells) {
        mainWindow->addRecentURL(url);
    }

}

void KoPart::setTitleModified(const QString &caption, bool mod)
{
    // we must be root doc so update caption in all related windows
    foreach(KoMainWindow *mainWindow, d->shells) {
        mainWindow->updateCaption(caption, mod);
        mainWindow->updateReloadFileAction(d->document);
        mainWindow->updateVersionsFileAction(d->document);
    }
}

void KoPart::slotStarted(KIO::Job *job)
{
    if (job && job->ui()) {
        job->ui()->setWindow(currentShell());
    }
}


void KoPart::showStartUpWidget(KoMainWindow *mainWindow, bool alwaysShow)
{
#ifndef NDEBUG
    if (d->templateType.isEmpty())
        kDebug(30003) << "showStartUpWidget called, but setTemplateType() never called. This will not show a lot";
#endif

    if (!alwaysShow) {
        KConfigGroup cfgGrp(componentData().config(), "TemplateChooserDialog");
        QString fullTemplateName = cfgGrp.readPathEntry("AlwaysUseTemplate", QString());
        if (!fullTemplateName.isEmpty()) {
            KUrl url(fullTemplateName);
            QFileInfo fi(url.toLocalFile());
            if (!fi.exists()) {
                QString appName = KGlobal::mainComponent().componentName();
                QString desktopfile = KGlobal::dirs()->findResource("data", appName + "/templates/*/" + fullTemplateName);
                if (desktopfile.isEmpty()) {
                    desktopfile = KGlobal::dirs()->findResource("data", appName + "/templates/" + fullTemplateName);
                }
                if (desktopfile.isEmpty()) {
                    fullTemplateName.clear();
                } else {
                    KUrl templateURL;
                    KDesktopFile f(desktopfile);
                    templateURL.setPath(KUrl(desktopfile).directory() + '/' + f.readUrl());
                    fullTemplateName = templateURL.toLocalFile();
                }
            }
            if (!fullTemplateName.isEmpty()) {
                openTemplate(fullTemplateName);
                shells().first()->setRootDocument(d->document, this);
                return;
            }
        }
    }

    mainWindow->factory()->container("mainToolBar", mainWindow)->hide();

    if (d->startUpWidget) {
        d->startUpWidget->show();
    } else {
        d->startUpWidget = createOpenPane(mainWindow, componentData(), d->templateType);
        mainWindow->setCentralWidget(d->startUpWidget);
    }

    mainWindow->setDocToOpen(this);
}

void KoPart::deleteOpenPane(bool closing)
{
    if (d->startUpWidget) {
        d->startUpWidget->hide();
        d->startUpWidget->deleteLater();

        if(!closing) {
            shells().first()->setRootDocument(d->document, this);
            KoPart::shells().first()->factory()->container("mainToolBar",
                                                                  shells().first())->show();
        }
    } else {
        emit closeEmbedInitDialog();
    }
}

QList<KoPart::CustomDocumentWidgetItem> KoPart::createCustomDocumentWidgets(QWidget * /*parent*/)
{
    return QList<CustomDocumentWidgetItem>();
}

void KoPart::setTemplateType(const QString& _templateType)
{
    d->templateType = _templateType;
}

QString KoPart::templateType() const
{
    return d->templateType;
}

void KoPart::startCustomDocument()
{
    deleteOpenPane();
}

KoOpenPane *KoPart::createOpenPane(QWidget *parent, const KComponentData &componentData,
                                       const QString& templateType)
{
    const QStringList mimeFilter = KoFilterManager::mimeFilter(KoServiceProvider::readNativeFormatMimeType(),
                                                               KoFilterManager::Import, KoServiceProvider::readExtraNativeMimeTypes());

    KoOpenPane *openPane = new KoOpenPane(parent, componentData, mimeFilter, templateType);
    QList<CustomDocumentWidgetItem> widgetList = createCustomDocumentWidgets(openPane);
    foreach(const CustomDocumentWidgetItem & item, widgetList) {
        openPane->addCustomDocumentWidget(item.widget, item.title, item.icon);
        connect(item.widget, SIGNAL(documentSelected()), this, SLOT(startCustomDocument()));
    }
    openPane->show();

    connect(openPane, SIGNAL(openExistingFile(const KUrl&)), this, SLOT(openExistingFile(const KUrl&)));
    connect(openPane, SIGNAL(openTemplate(const KUrl&)), this, SLOT(openTemplate(const KUrl&)));

    return openPane;
}


#include <KoPart.moc>
