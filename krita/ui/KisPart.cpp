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
#include <KoIcon.h>

#include "KisApplication.h"
#include "KisMainWindow.h"
#include "KisDocument.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "KisOpenPane.h"
#include "KisImportExportManager.h"
#include "dialogs/KisShortcutsDialog.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kxmlguifactory.h>
#include <knotification.h>
#include <kdialog.h>
#include <kdesktopfile.h>
#include <QMessageBox>
#include <kmimetype.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kshortcut.h>

#include <QDialog>
#include <QGraphicsScene>
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QDomDocument>
#include <QDomElement>

#include "KisView.h"
#include "KisDocument.h"
#include "kis_factory2.h"
#include "kis_config.h"
#include "kis_clipboard.h"
#include "kis_custom_image_widget.h"
#include "kis_image_from_clipboard_widget.h"
#include "kis_shape_controller.h"
#include "kis_resource_server_provider.h"

#include "kis_color_manager.h"

#include "kis_action.h"

class KisPart::Private
{
public:
    Private(KisPart *_part)
        : part(_part)
        , canvasItem(0)
        , startupWidget(0)
        , actionCollection(0)
    {
    }

    ~Private()
    {
        delete canvasItem;
    }

    KisPart *part;

    QList<QPointer<KisView> > views;
    QList<QPointer<KisMainWindow> > mainWindows;
    QList<QPointer<KisDocument> > documents;
    QGraphicsItem *canvasItem;
    QString templatesResourcePath;
    KisOpenPane *startupWidget;

    KActionCollection *actionCollection;

    void loadActions();

};


void KisPart::Private::loadActions()
{
    actionCollection = new KActionCollection(part, KisFactory::componentName());

    KGlobal::dirs()->addResourceType("kis_actions", "data", "krita/actions/");
    QStringList actionDefinitions = KGlobal::dirs()->findAllResources("kis_actions", "*.action", KStandardDirs::Recursive | KStandardDirs::NoDuplicates);

    foreach(const QString &actionDefinition, actionDefinitions)  {
        QDomDocument doc;
        QFile f(actionDefinition);
        f.open(QFile::ReadOnly);
        doc.setContent(f.readAll());

        QDomElement e = doc.documentElement(); // Actions
        QString collection = e.attribute("name");

        e = e.firstChild().toElement(); // Action

        while (!e.isNull()) {
            if (e.tagName() == "Action") {
                QString name = e.attribute("name");

                QString icon = e.attribute("icon");
                QString text = i18n(e.attribute("text").toUtf8().constData());
                QString whatsthis = i18n(e.attribute("whatsThis").toUtf8().constData());
                QString toolTip = i18n(e.attribute("toolTip").toUtf8().constData());
                QString statusTip = i18n(e.attribute("statusTip").toUtf8().constData());
                QString iconText = i18n(e.attribute("iconText").toUtf8().constData());
                KShortcut shortcut = KShortcut(e.attribute("shortcut"));
                bool isCheckable = e.attribute("isCheckable") == "true" ? true : false;
                KShortcut defaultShortcut = KShortcut(e.attribute("defaultShortcut"));

                if (name.isEmpty()) {
                    qDebug() << text << "has no name! From:" << actionDefinition;
                }

                KisAction *action = new KisAction(KIcon(icon), text);
                action->setObjectName(name);
                action->setWhatsThis(whatsthis);
                action->setToolTip(toolTip);
                action->setStatusTip(statusTip);
                action->setIconText(iconText);
                action->setShortcut(shortcut, KAction::ActiveShortcut);
                action->setCheckable(isCheckable);
                action->setShortcut(defaultShortcut, KAction::DefaultShortcut);

                if (!actionCollection->action(name)) {
                    actionCollection->addAction(name, action);
                }
//                else {
//                    qDebug() << "duplicate action" << name << action << "from" << collection;
//                    delete action;
//                }
            }
            e = e.nextSiblingElement();
        }
        actionCollection->readSettings();
    }

    //check for colliding shortcuts
    QMap<QKeySequence, QAction*> existingShortcuts;
    foreach(QAction* action, actionCollection->actions()) {
        if(action->shortcut() == QKeySequence(0)) {
            continue;
        }
        if (existingShortcuts.contains(action->shortcut())) {
            qDebug() << "action" << action->text() << "and" <<  existingShortcuts[action->shortcut()]->text() << "have the same shortcut:" << action->shortcut();
        }
        else {
            existingShortcuts[action->shortcut()] = action;
        }
    }

}

KisPart* KisPart::instance()
{
    K_GLOBAL_STATIC(KisPart, s_instance)
    return s_instance;
}


KisPart::KisPart()
    : d(new Private(this))
{
    setTemplatesResourcePath(QLatin1String("krita/templates/"));

    // Preload all the resources in the background
    Q_UNUSED(KoResourceServerProvider::instance());
    Q_UNUSED(KisResourceServerProvider::instance());
    Q_UNUSED(KisColorManager::instance());

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
        emit documentOpened('/'+objectName());
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


int KisPart::documentCount() const
{
    return d->documents.size();
}

void KisPart::removeDocument(KisDocument *document)
{
    d->documents.removeAll(document);
    emit documentClosed('/'+objectName());
    document->deleteLater();
}

KisMainWindow *KisPart::createMainWindow()
{
    KisMainWindow *mw = new KisMainWindow();

    addMainWindow(mw);

    return mw;
}

KisView *KisPart::createView(KisDocument *document, KoCanvasResourceManager *resourceManager, KActionCollection *actionCollection, QWidget *parent)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    KisView *view  = new KisView(document, resourceManager, actionCollection, parent);
    QApplication::restoreOverrideCursor();

    addView(view);

    return view;
}

void KisPart::addView(KisView *view)
{
    if (!view)
        return;

    if (!d->views.contains(view)) {
        d->views.append(view);
    }

    connect(view, SIGNAL(destroyed()), this, SLOT(viewDestroyed()));

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

    KisView *view = createView(document, 0, 0, 0);
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

    KisView *view = createView(document, mw->resourceManager(), mw->actionCollection(), mw);
    mw->addView(view);

    if (d->startupWidget) {
        d->startupWidget->setParent(0);
        d->startupWidget->hide();
    }
    qApp->restoreOverrideCursor();
}

void KisPart::configureShortcuts()
{
    if (!d->actionCollection) {
        d->loadActions();
    }

    // In kdelibs4 a hack was used to hide the shortcut schemes widget in the
    // normal shortcut editor KShortcutsDialog from kdelibs, by setting the
    // bottom buttons oneself. This does not work anymore (as the buttons are
    // no longer exposed), so for now running with a plain copy of the sources
    // of KShortcutsDialog, where the schemes editor is disabled directly.
    // Not nice, but then soon custom Krita-specific shortcut handling is
    // planned anyway.
    KisShortcutsDialog dlg(KShortcutsEditor::WidgetAction | KShortcutsEditor::WindowAction | KShortcutsEditor::ApplicationAction);
    dlg.addCollection(d->actionCollection);
    dlg.configure();

    foreach(KisMainWindow *mainWindow, d->mainWindows) {
        KActionCollection *ac = mainWindow->actionCollection();
        ac->readSettings();

        // append shortcuts to tooltips if they exist
        foreach( QAction* tempAction, ac->actions())
        {
            // find the shortcut pattern and delete (note the preceding space in the RegEx)
            QString strippedTooltip = tempAction->toolTip().remove(QRegExp("\\s\\(.*\\)"));

            // append shortcut if it exists for action
            if(tempAction->shortcut() == QKeySequence(0))
                 tempAction->setToolTip( strippedTooltip);
            else
                 tempAction->setToolTip( strippedTooltip + " (" + tempAction->shortcut().toString() + ")");

        }


    }
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
    KisView *view = createView(document, mw->resourceManager(), mw->actionCollection(), mw);
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

#ifndef NDEBUG
    if (d->templatesResourcePath.isEmpty())
        kDebug(30003) << "showStartUpWidget called, but setTemplatesResourcePath() never called. This will not show a lot";
#endif

    if (!alwaysShow) {
        KConfigGroup cfgGrp(KisFactory::componentData().config(), "TemplateChooserDialog");
        QString fullTemplateName = cfgGrp.readPathEntry("AlwaysUseTemplate", QString());
        if (!fullTemplateName.isEmpty()) {
            KUrl url(fullTemplateName);
            QFileInfo fi(url.toLocalFile());
            if (!fi.exists()) {
                const QString templatesResourcePath = this->templatesResourcePath();
                QString desktopfile = KGlobal::dirs()->findResource("data", templatesResourcePath + "*/" + fullTemplateName);
                if (desktopfile.isEmpty()) {
                    desktopfile = KGlobal::dirs()->findResource("data", templatesResourcePath + fullTemplateName);
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

    if (d->startupWidget) {
        delete d->startupWidget;
    }
    const QStringList mimeFilter = koApp->mimeFilter(KisImportExportManager::Import);

    d->startupWidget = new KisOpenPane(0, KisFactory::componentData(), mimeFilter, d->templatesResourcePath);
    d->startupWidget->setWindowModality(Qt::WindowModal);
    QList<CustomDocumentWidgetItem> widgetList = createCustomDocumentWidgets(d->startupWidget);
    foreach(const CustomDocumentWidgetItem & item, widgetList) {
        d->startupWidget->addCustomDocumentWidget(item.widget, item.title, item.icon);
        connect(item.widget, SIGNAL(documentSelected(KisDocument*)), this, SLOT(startCustomDocument(KisDocument*)));
    }

    connect(d->startupWidget, SIGNAL(openExistingFile(const KUrl&)), this, SLOT(openExistingFile(const KUrl&)));
    connect(d->startupWidget, SIGNAL(openTemplate(const KUrl&)), this, SLOT(openTemplate(const KUrl&)));

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

    return widgetList;
}

void KisPart::setTemplatesResourcePath(const QString &templatesResourcePath)
{
    Q_ASSERT(!templatesResourcePath.isEmpty());
    Q_ASSERT(templatesResourcePath.endsWith(QLatin1Char('/')));

    d->templatesResourcePath = templatesResourcePath;
}

QString KisPart::templatesResourcePath() const
{
    return d->templatesResourcePath;
}

void KisPart::startCustomDocument(KisDocument* doc)
{
    addDocument(doc);
    KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->startupWidget->parent());
    if (!mw) mw = currentMainwindow();
    KisView *view = createView(doc, mw->resourceManager(), mw->actionCollection(), mw);
    mw->addView(view);

    d->startupWidget->setParent(0);
    d->startupWidget->hide();
}

KisInputManager* KisPart::currentInputManager()
{
    return instance()->currentMainwindow()->viewManager()->inputManager();
}


#include <KisPart.moc>
