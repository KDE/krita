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

#include "KisPart.h"

#include "KoProgressProxy.h"
#include <KoCanvasController.h>
#include <KoCanvasControllerWidget.h>
#include <KoColorSpaceEngine.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoInteractionTool.h>
#include <KoShapeBasedDocumentBase.h>
#include <KoResourceServerProvider.h>

#include "KisApplication.h"
#include "KisMainWindow.h"
#include "KisDocument.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "KisOpenPane.h"
#include "KisImportExportManager.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kxmlguifactory.h>
#include <kdeprintdialog.h>
#include <knotification.h>
#include <kdialog.h>
#include <kdesktopfile.h>
#include <QMessageBox>
#include <kmimetype.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QDialog>
#include <QGraphicsScene>
#include <QApplication>
#include <QGraphicsProxyWidget>

#include "KisView.h"
#include "KisDocument.h"
#include "kis_factory2.h"
#include "kis_config.h"
#include "kis_clipboard.h"
#include "kis_custom_image_widget.h"
#include "kis_image_from_clipboard_widget.h"
#include "kis_shape_controller.h"
#include "kis_resource_server_provider.h"
#include "kis_animation_selector.h"
#include "kis_animation_doc.h"


class KisPart::Private
{
public:
    Private()
        : canvasItem(0)
        , startupWidget(0)
    {
    }

    ~Private()
    {
        delete canvasItem;
    }

    QList<QPointer<KisView> > views;
    QList<QPointer<KisMainWindow> > mainWindows;
    QList<QPointer<KisDocument> > documents;
    QGraphicsItem *canvasItem;
    QString templateType;
    KisOpenPane *startupWidget;

};

KisPart* KisPart::instance()
{
    K_GLOBAL_STATIC(KisPart, s_instance)
    return s_instance;
}


KisPart::KisPart()
    : d(new Private())
{
    setTemplateType("krita_template");

    // Preload all the resources in the background
    Q_UNUSED(KoResourceServerProvider::instance());
    Q_UNUSED(KisResourceServerProvider::instance());

    m_dieOnError = false;
}

KisPart::~KisPart()
{
    while (!d->documents.isEmpty()) {
        delete d->documents.takeFirst();
    }

    while (!d->views.isEmpty()) {
        delete d->views.takeFirst();
    }

    while (!d->mainWindows.isEmpty()) {
        delete d->mainWindows.takeFirst();
    }

    delete d;
}

void KisPart::addDocument(KisDocument *document)
{
    //qDebug() << "Adding document to part list" << document;
    Q_ASSERT(document);
    if (!d->documents.contains(document)) {
        d->documents.append(document);
    }
}

QList<QPointer<KisDocument> > KisPart::documents() const
{
    return d->documents;
}

KisDocument *KisPart::createDocument() const
{
    KisDocument *doc = new KisDocument();
    return doc;
}

KisAnimationDoc *KisPart::createAnimationDoc() const
{
    return new KisAnimationDoc();
}



int KisPart::documentCount() const
{
    return d->documents.size();
}

void KisPart::removeDocument(KisDocument *document)
{
    d->documents.removeAll(document);
    document->deleteLater();
}

KisMainWindow *KisPart::createMainWindow()
{
    KisMainWindow *mw = new KisMainWindow();
    return mw;
}

KisView *KisPart::createView(KisDocument *document, KisMainWindow *parent)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    KisView *view  = new KisView(document, parent->resourceManager(), parent->actionCollection(), parent);

    // XXX: this prevents a crash when opening a new document after opening a
    // a document that has not been touched! I have no clue why, though.
    // see: https://bugs.kde.org/show_bug.cgi?id=208239.
    document->setModified(true);
    document->setModified(false);
    QApplication::restoreOverrideCursor();

    addView(view, document);

    return view;
}

void KisPart::addView(KisView *view, KisDocument *document)
{
    if (!view)
        return;

    if (!d->views.contains(view)) {
        d->views.append(view);
    }

    if (!d->documents.contains(document)) {
        d->documents.append(document);
    }

    connect(view, SIGNAL(destroyed()), this, SLOT(viewDestroyed()));

    if (d->views.size() == 1) {
        KisApplication *app = qobject_cast<KisApplication*>(KApplication::kApplication());
        if (0 != app) {
            emit app->documentOpened('/'+objectName());
        }
    }

    emit sigViewAdded(view);
}

void KisPart::removeView(KisView *view)
{
    if (!view) return;

    emit sigViewRemoved(view);

    QPointer<KisDocument> doc = view->document();
    d->views.removeAll(view);

    if (doc) {
        bool found = false;
        foreach(QPointer<KisView> view, d->views) {
            if (view && view->document() == doc) {
                found = true;
                break;
            }
        }
        if (!found) {
            removeDocument(doc);
        }
    }

    if (d->views.isEmpty()) {
        KisApplication *app = qobject_cast<KisApplication*>(KApplication::kApplication());
        if (0 != app) {
            emit app->documentClosed('/'+objectName());
        }
    }
}

QList<QPointer<KisView> > KisPart::views() const
{
    return d->views;
}

int KisPart::viewCount(KisDocument *doc) const
{
    if (!doc) {
        return d->views.count();
    }
    else {
        int count = 0;
        foreach(QPointer<KisView> view, d->views) {
            if (view->document() == doc) {
                count++;
            }
        }
        return count;
    }
}

QGraphicsItem *KisPart::canvasItem(KisDocument *document, bool create)
{
    if (create && !d->canvasItem) {
        d->canvasItem = createCanvasItem(document);
    }
    return d->canvasItem;
}

QGraphicsItem *KisPart::createCanvasItem(KisDocument *document)
{
    if (!document) return 0;

    KisView *view = createView(document, 0);
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    QWidget *canvasController = view->findChild<KoCanvasControllerWidget*>();
    proxy->setWidget(canvasController);
    return proxy;
}

void KisPart::addMainWindow(KisMainWindow *mainWindow)
{
    if (!mainWindow) return;
    if (d->mainWindows.contains(mainWindow)) return;

    kDebug(30003) <<"mainWindow" << (void*)mainWindow <<"added to doc" << this;
    d->mainWindows.append(mainWindow);

}

void KisPart::removeMainWindow(KisMainWindow *mainWindow)
{
    kDebug(30003) <<"mainWindow" << (void*)mainWindow <<"removed from doc" << this;
    if (mainWindow) {
        d->mainWindows.removeAll(mainWindow);
    }
}

const QList<QPointer<KisMainWindow> > &KisPart::mainWindows() const
{
    return d->mainWindows;
}

int KisPart::mainwindowCount() const
{
    return d->mainWindows.count();
}


KisMainWindow *KisPart::currentMainwindow() const
{
    QWidget *widget = qApp->activeWindow();
    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(widget);
    while (!mainWindow && widget) {
        widget = widget->parentWidget();
        mainWindow = qobject_cast<KisMainWindow*>(widget);
    }

    if (!mainWindow && mainWindows().size() > 0) {
        mainWindow = mainWindows().first();
    }
    return mainWindow;

}

void KisPart::openExistingFile(const KUrl& url)
{
    qApp->setOverrideCursor(Qt::BusyCursor);
    KisDocument *document = createDocument();
    if (!document->openUrl(url)) {
        return;
    }
    document->setModified(false);
    addDocument(document);

    KisMainWindow *mw = 0;
    if (d->startupWidget) {
        mw = qobject_cast<KisMainWindow*>(d->startupWidget->parent());
    }
    if (!mw) {
        mw = currentMainwindow();
    }

    KisView *view = createView(document, mw);
    mw->addView(view);

    if (d->startupWidget) {
        d->startupWidget->setParent(0);
        d->startupWidget->hide();
    }
    qApp->restoreOverrideCursor();
}

void KisPart::openTemplate(const KUrl& url)
{
    qApp->setOverrideCursor(Qt::BusyCursor);
    KisDocument *document = createDocument();

    bool ok = document->loadNativeFormat(url.toLocalFile());
    document->setModified(false);
    document->undoStack()->clear();

    if (ok) {
        QString mimeType = KMimeType::findByUrl( url, 0, true )->name();
        // in case this is a open document template remove the -template from the end
        mimeType.remove( QRegExp( "-template$" ) );
        document->setMimeTypeAfterLoading(mimeType);
        document->resetURL();
        document->setEmpty();
    } else {
        document->showLoadingErrorDialog();
        document->initEmpty();
    }
    addDocument(document);

    KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->startupWidget->parent());
    if (!mw) mw = currentMainwindow();
    KisView *view = createView(document, mw);
    mw->addView(view);

    d->startupWidget->setParent(0);
    d->startupWidget->hide();
    qApp->restoreOverrideCursor();
}

void KisPart::viewDestroyed()
{
    KisView *view = qobject_cast<KisView*>(sender());
    if (view) {
        removeView(view);
    }
}

void KisPart::addRecentURLToAllMainWindows(KUrl url)
{
    // Add to recent actions list in our mainWindows
    foreach(KisMainWindow *mainWindow, d->mainWindows) {
        mainWindow->addRecentURL(url);
    }
}

void KisPart::showStartUpWidget(KisMainWindow *mainWindow, bool alwaysShow)
{
    // print error if the lcms engine is not available
    if (!KoColorSpaceEngineRegistry::instance()->contains("icc")) {
        // need to wait 1 event since exiting here would not work.
        m_errorMessage = i18n("The Calligra LittleCMS color management plugin is not installed. Krita will quit now.");
        m_dieOnError = true;
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
    }

#ifndef NDEBUG
    if (d->templateType.isEmpty())
        kDebug(30003) << "showStartUpWidget called, but setTemplateType() never called. This will not show a lot";
#endif

    if (!alwaysShow) {
        KConfigGroup cfgGrp(KisFactory::componentData().config(), "TemplateChooserDialog");
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
                return;
            }
        }
    }

    if (!d->startupWidget) {
        const QStringList mimeFilter = koApp->mimeFilter(KisImportExportManager::Import);

        d->startupWidget = new KisOpenPane(0, KisFactory::componentData(), mimeFilter, d->templateType);
        d->startupWidget->setWindowModality(Qt::WindowModal);
        QList<CustomDocumentWidgetItem> widgetList = createCustomDocumentWidgets(d->startupWidget);
        foreach(const CustomDocumentWidgetItem & item, widgetList) {
            d->startupWidget->addCustomDocumentWidget(item.widget, item.title, item.icon);
            connect(item.widget, SIGNAL(documentSelected(KisDocument*)), this, SLOT(startCustomDocument(KisDocument*)));
        }

        connect(d->startupWidget, SIGNAL(openExistingFile(const KUrl&)), this, SLOT(openExistingFile(const KUrl&)));
        connect(d->startupWidget, SIGNAL(openTemplate(const KUrl&)), this, SLOT(openTemplate(const KUrl&)));

    }

    d->startupWidget->setParent(mainWindow);
    d->startupWidget->setWindowFlags(Qt::Dialog);
    d->startupWidget->exec();
}

QList<KisPart::CustomDocumentWidgetItem> KisPart::createCustomDocumentWidgets(QWidget * parent)
{
    KisConfig cfg;

    int w = cfg.defImageWidth();
    int h = cfg.defImageHeight();

    QList<KisPart::CustomDocumentWidgetItem> widgetList;
    {
        KisPart::CustomDocumentWidgetItem item;
        item.widget = new KisCustomImageWidget(parent,
                                               w, h, cfg.defImageResolution(), cfg.defColorModel(), cfg.defaultColorDepth(), cfg.defColorProfile(),
                                               i18n("Unnamed"));

        item.icon = "application-x-krita";
        widgetList << item;
    }

    {
        QSize sz = KisClipboard::instance()->clipSize();
        if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
            w = sz.width();
            h = sz.height();
        }

        KisPart::CustomDocumentWidgetItem item;
        item.widget = new KisImageFromClipboard(parent,
                                                w, h, cfg.defImageResolution(), cfg.defColorModel(), cfg.defaultColorDepth(), cfg.defColorProfile(),
                                                i18n("Unnamed"));

        item.title = i18n("Create from Clipboard");
        item.icon = "klipper";

        widgetList << item;


    }
#if 0
    {
        KisPart::CustomDocumentWidgetItem item;
        item.widget = new KisAnimationSelector(parent, w, h, cfg.defImageResolution(), cfg.defColorModel(), cfg.defaultColorDepth(), cfg.defColorProfile(),
                                               i18n("untitled-animation"));

        item.title = i18n("Animation");
        item.icon = "tool-animator";
        widgetList << item;
    }
#endif

    return widgetList;
}

void KisPart::setTemplateType(const QString& _templateType)
{
    d->templateType = _templateType;
}

QString KisPart::templateType() const
{
    return d->templateType;
}

void KisPart::startCustomDocument(KisDocument* doc)
{
    addDocument(doc);
    KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->startupWidget->parent());
    if (!mw) mw = currentMainwindow();
    KisView *view = createView(doc, mw);
    mw->addView(view);

    d->startupWidget->setParent(0);
    d->startupWidget->hide();
}



void KisPart::showErrorAndDie()
{
    QMessageBox::critical(0,
                          i18n("Installation error"),
                          m_errorMessage);
    if (m_dieOnError) {
        exit(10);
    }
}

#include <KisPart.moc>
