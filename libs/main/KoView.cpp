/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoView.h"

// local directory
#include "KoView_p.h"

#include "KoPart.h"
#include "KoDockRegistry.h"
#include "KoDocument.h"
#include "KoMainWindow.h"
#include "KoViewAdaptor.h"
#include "KoDockFactoryBase.h"
#include "KoUndoStackAction.h"
#include "KoGlobal.h"
#include "KoPageLayout.h"
#include "KoPrintJob.h"

#include <KoIcon.h>

#include <kactioncollection.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kparts/partmanager.h>
#include <kparts/event.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>
#include <kselectaction.h>
#include <kconfiggroup.h>
#include <kdeprintdialog.h>

#include <QTimer>
#include <QDockWidget>
#include <QToolBar>
#include <QApplication>
#include <QList>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QImage>
#include <QUrl>
#include <QPrintDialog>

//static
QString KoView::newObjectName()
{
    static int s_viewIFNumber = 0;
    QString name; name.setNum(s_viewIFNumber++); name.prepend("view_");
    return name;
}

class KoViewPrivate
{
public:
    KoViewPrivate() {
        manager = 0;
        tempActiveWidget = 0;
        registered = false;
        documentDeleted = false;
        viewBar = 0;
    }
    ~KoViewPrivate() {
    }

    QPointer<KoDocument> document; // our KoDocument
    KoPart *part; // our part
    QPointer<KParts::PartManager> manager;
    QWidget *tempActiveWidget;
    bool registered;  // are we registered at the part manager?
    bool documentDeleted; // true when document gets deleted [can't use document==0
    // since this only happens in ~QObject, and views
    // get deleted by ~KoDocument].
    QTimer *scrollTimer;


    // Hmm sorry for polluting the private class with such a big inner class.
    // At the beginning it was a little struct :)
    class StatusBarItem
    {
    public:
        StatusBarItem() // for QValueList
            : m_widget(0),
              m_connected(false),
              m_hidden(false) {}

        StatusBarItem(QWidget * widget, int stretch, bool permanent)
            : m_widget(widget),
              m_stretch(stretch),
              m_permanent(permanent),
              m_connected(false),
              m_hidden(false) {}

        bool operator==(const StatusBarItem& rhs) {
            return m_widget == rhs.m_widget;
        }

        bool operator!=(const StatusBarItem& rhs) {
            return m_widget != rhs.m_widget;
        }

        QWidget * widget() const {
            return m_widget;
        }

        void ensureItemShown(KStatusBar * sb) {
            Q_ASSERT(m_widget);
            if (!m_connected) {
                if (m_permanent)
                    sb->addPermanentWidget(m_widget, m_stretch);
                else
                    sb->addWidget(m_widget, m_stretch);

                if(!m_hidden)
                    m_widget->show();

                m_connected = true;
            }
        }
        void ensureItemHidden(KStatusBar * sb) {
            if (m_connected) {
                m_hidden = m_widget->isHidden();
                sb->removeWidget(m_widget);
                m_widget->hide();
                m_connected = false;
            }
        }
    private:
        QWidget * m_widget;
        int m_stretch;
        bool m_permanent;
        bool m_connected;
        bool m_hidden;
    };

    QList<StatusBarItem> statusBarItems; // Our statusbar items
    bool inOperation; //in the middle of an operation (no screen refreshing)?
    QToolBar* viewBar;
};

KoView::KoView(KoPart *part, KoDocument *document, QWidget *parent)
        : QWidget(parent)
        , d(new KoViewPrivate)
{
    Q_ASSERT(document);
    Q_ASSERT(part);

    setObjectName(newObjectName());

    new KoViewAdaptor(this);
    QDBusConnection::sessionBus().registerObject('/' + objectName(), this);

    //kDebug(30003) <<"KoView::KoView" << this;
    d->document = document;
    d->part = part;

    KParts::PartBase::setPartObject(this);

    setFocusPolicy(Qt::StrongFocus);

    setupGlobalActions();

    KStatusBar * sb = statusBar();
    if (sb) { // No statusbar in e.g. konqueror
        connect(d->document, SIGNAL(statusBarMessage(const QString&)),
                this, SLOT(slotActionStatusText(const QString&)));
        connect(d->document, SIGNAL(clearStatusBarMessage()),
                this, SLOT(slotClearStatusText()));
    }

    d->scrollTimer = new QTimer(this);
    connect(d->scrollTimer, SIGNAL(timeout()), this, SLOT(slotAutoScroll()));

    // add all plugins.
    foreach(const QString & docker, KoDockRegistry::instance()->keys()) {
        KoDockFactoryBase *factory = KoDockRegistry::instance()->value(docker);
        if (shell())
            shell()->createDockWidget(factory);
    }

    actionCollection()->addAssociatedWidget(this);
    foreach(QAction* action, actionCollection()->actions())
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

KoView::~KoView()
{
    kDebug(30003) << "KoView::~KoView" << this;
    delete d->scrollTimer;
    if (!d->documentDeleted) {
        if (d->document) {
            if (d->manager && d->registered)   // if we aren't registered we mustn't unregister :)
                d->manager->removePart(d->part);
            d->part->removeView(this);
        }
    }
    delete d;
}


void KoView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasUrls()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KoView::dropEvent(QDropEvent *event)
{
    // we can drop a list of urls from, for instance dolphin
    QList<QImage> images;

    if (event->mimeData()->hasImage()) {
        images << event->mimeData()->imageData().value<QImage>();
    }
    else if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        foreach (const QUrl url, urls) {
            QImage image;
            KUrl kurl(url);
            // make sure we download the files before inserting them
            if (!kurl.isLocalFile()) {
                QString tmpFile;
                if( KIO::NetAccess::download(kurl, tmpFile, this)) {
                    image.load(tmpFile);
                    KIO::NetAccess::removeTempFile(tmpFile);
                } else {
                    KMessageBox::error(this, KIO::NetAccess::lastErrorString());
                }
            }
            else {
                image.load(kurl.toLocalFile());
            }
            if (!image.isNull()) {
                images << image;
            }
        }
    }

    if (!images.isEmpty()) {
        addImages(images, event->pos());
    }
}


void KoView::addImages(const QList<QImage> &, const QPoint &)
{
    // override in your application
}

KoDocument *KoView::koDocument() const
{
    return d->document;
}

void KoView::setDocumentDeleted()
{
    d->documentDeleted = true;
}

bool KoView::documentDeleted() const
{
    return d->documentDeleted;
}

void KoView::setPartManager(KParts::PartManager *manager)
{
    d->manager = manager;
    if (!manager->parts().contains(d->part)) {  // is there another view registered?
        d->registered = true; // no, so we have to register now and ungregister again in the DTOR
        manager->addPart(d->part, false);
    } else
        d->registered = false;  // There is already another view registered for that part...
}

KParts::PartManager *KoView::partManager() const
{
    return d->manager;
}

QAction *KoView::action(const QDomElement &element) const
{
    static const QString &attrName = KGlobal::staticQString("name");
    QString name = element.attribute(attrName);

    QAction *act = KXMLGUIClient::action(name.toUtf8());

    if (!act)
        act = d->part->KXMLGUIClient::action(name.toUtf8());

    // last resort, try to get action from the main window if there is one
    if (!act && shell())
        act = shell()->actionCollection()->action(name);

    return act;
}

QAction *KoView::action(const char* name) const
{
    QAction *act = KXMLGUIClient::action(name);

    if (!act)
        act = d->part->KXMLGUIClient::action(name);

    // last resort, try to get action from the main window if there is one
    if (!act && shell())
        act = shell()->actionCollection()->action(name);

    return act;
}

KoDocument *KoView::hitTest(const QPoint &viewPos)
{
    Q_UNUSED(viewPos);
    return d->document; // we no longer have child documents
}

int KoView::leftBorder() const
{
    return 0;
}

int KoView::rightBorder() const
{
    return 0;
}

int KoView::topBorder() const
{
    return 0;
}

int KoView::bottomBorder() const
{
    return 0;
}

QWidget *KoView::canvas() const
{
    //dfaure: since the view plays two roles in this method (the const means "you can modify the canvas
    // but not the view", it's just coincidence that the view is the canvas by default ;)
    return const_cast<KoView *>(this);
}

int KoView::canvasXOffset() const
{
    return 0;
}

int KoView::canvasYOffset() const
{
    return 0;
}

void KoView::customEvent(QEvent *ev)
{
    if (KParts::PartActivateEvent::test(ev))
        partActivateEvent((KParts::PartActivateEvent *)ev);
    else if (KParts::PartSelectEvent::test(ev))
        partSelectEvent((KParts::PartSelectEvent *)ev);
    else if (KParts::GUIActivateEvent::test(ev))
        guiActivateEvent((KParts::GUIActivateEvent*)ev);
}

void KoView::partActivateEvent(KParts::PartActivateEvent *event)
{
    emit activated(event->activated());
}

void KoView::partSelectEvent(KParts::PartSelectEvent *event)
{
    emit selected(event->selected());
}

void KoView::guiActivateEvent(KParts::GUIActivateEvent * ev)
{
    showAllStatusBarItems(ev->activated());
}

void KoView::showAllStatusBarItems(bool show)
{
    KStatusBar * sb = statusBar();
    if (!sb)
        return;
    int itemCount = d->statusBarItems.count();
    for (int i = 0; i < itemCount; ++i) {
        KoViewPrivate::StatusBarItem &sbItem = d->statusBarItems[i];
        if (show) {
            sbItem.ensureItemShown(sb);
        } else {
            sbItem.ensureItemHidden(sb);
        }
    }
}

void KoView::addStatusBarItem(QWidget * widget, int stretch, bool permanent)
{
    KoViewPrivate::StatusBarItem item(widget, stretch, permanent);
    KStatusBar * sb = statusBar();
    if (sb) {
        item.ensureItemShown(sb);
    }
    d->statusBarItems.append(item);
}

void KoView::removeStatusBarItem(QWidget *widget)
{
    KStatusBar *sb = statusBar();

    int itemCount = d->statusBarItems.count();
    for (int i = itemCount-1; i >= 0; --i) {
        KoViewPrivate::StatusBarItem &sbItem = d->statusBarItems[i];
        if (sbItem.widget() == widget) {
            if (sb) {
                sbItem.ensureItemHidden(sb);
            }
            d->statusBarItems.removeOne(sbItem);
            break;
        }
    }
}

void KoView::enableAutoScroll()
{
    d->scrollTimer->start(50);
}

void KoView::disableAutoScroll()
{
    d->scrollTimer->stop();
}

int KoView::autoScrollAcceleration(int offset) const
{
    if (offset < 40)
        return offset;
    else
        return offset*offset / 40;
}

void KoView::slotAutoScroll()
{
    QPoint scrollDistance;
    bool actuallyDoScroll = false;
    QPoint pos(mapFromGlobal(QCursor::pos()));

    //Provide progressive scrolling depending on the mouse position
    if (pos.y() < topBorder()) {
        scrollDistance.setY((int) - autoScrollAcceleration(- pos.y() + topBorder()));
        actuallyDoScroll = true;
    } else if (pos.y() > height() - bottomBorder()) {
        scrollDistance.setY((int) autoScrollAcceleration(pos.y() - height() + bottomBorder()));
        actuallyDoScroll = true;
    }

    if (pos.x() < leftBorder()) {
        scrollDistance.setX((int) - autoScrollAcceleration(- pos.x() + leftBorder()));
        actuallyDoScroll = true;
    } else if (pos.x() > width() - rightBorder()) {
        scrollDistance.setX((int) autoScrollAcceleration(pos.x() - width() + rightBorder()));
        actuallyDoScroll = true;
    }

    if (actuallyDoScroll) {
        pos = canvas()->mapFrom(this, pos);
        QMouseEvent* event = new QMouseEvent(QEvent::MouseMove, pos, Qt::NoButton, Qt::NoButton,
                                             QApplication::keyboardModifiers());

        QApplication::postEvent(canvas(), event);
        emit autoScroll(scrollDistance);
    }
}

KoPrintJob * KoView::createPrintJob()
{
    kWarning(30003) << "Printing not implemented in this application";
    return 0;
}

KoPrintJob * KoView::createPdfPrintJob()
{
    return createPrintJob();
}

KoPageLayout KoView::pageLayout() const
{
    return koDocument()->pageLayout();
}

QPrintDialog *KoView::createPrintDialog(KoPrintJob *printJob, QWidget *parent)
{
    QPrintDialog *printDialog = KdePrint::createPrintDialog(&printJob->printer(),
                                printJob->createOptionWidgets(), parent);
    printDialog->setMinMax(printJob->printer().fromPage(), printJob->printer().toPage());
    printDialog->setEnabledOptions(printJob->printDialogOptions());
    return printDialog;
}

void KoView::setupGlobalActions()
{
    KAction *actionNewView  = new KAction(koIcon("window-new"), i18n("&New View"), this);
    actionCollection()->addAction("view_newview", actionNewView);
    connect(actionNewView, SIGNAL(triggered(bool)), this, SLOT(newView()));

    actionCollection()->addAction("edit_undo", new KoUndoStackAction(d->document->undoStack(), KoUndoStackAction::UNDO));
    actionCollection()->addAction("edit_redo", new KoUndoStackAction(d->document->undoStack(), KoUndoStackAction::RED0));

    KSelectAction *actionAuthor  = new KSelectAction(koIcon("user-identity"), i18n("Active Author Profile"), this);
    actionAuthor->addAction(i18n("Default Author Profile"));
    actionAuthor->addAction(i18nc("choice for author profile", "Anonymous"));
    KConfig *config = KoGlobal::calligraConfig();
    KConfigGroup authorGroup(config, "Author");
    QStringList profiles = authorGroup.readEntry("profile-names", QStringList());

    foreach (const QString &profile , profiles) {
        actionAuthor->addAction(profile);
    }
    actionCollection()->addAction("settings_active_author", actionAuthor);

    KConfigGroup appAuthorGroup(KGlobal::config(), "Author");
    QString profileName = appAuthorGroup.readEntry("active-profile", "");
    if (profileName == "anonymous") {
        actionAuthor->setCurrentItem(1);
    } else if (profiles.contains(profileName)) {
        actionAuthor->setCurrentAction(profileName);
    } else {
        actionAuthor->setCurrentItem(0);
    }

    connect(actionAuthor, SIGNAL(triggered(const QString &)), this, SLOT(changeAuthorProfile(const QString &)));
}

void KoView::newView()
{
    Q_ASSERT((d != 0 && d->document && d->part));

    KoDocument *thisDocument = d->document;
    KoMainWindow *shell = new KoMainWindow(d->part->componentData());
    shell->setRootDocument(thisDocument, d->part);
    shell->show();
}

void KoView::changeAuthorProfile(const QString &profileName)
{
    KConfigGroup appAuthorGroup(KGlobal::config(), "Author");
    if (profileName.isEmpty()) {
        appAuthorGroup.writeEntry("active-profile", "");
    } else if (profileName == i18nc("choice for author profile", "Anonymous")) {
        appAuthorGroup.writeEntry("active-profile", "anonymous");
    } else {
        appAuthorGroup.writeEntry("active-profile", profileName);
    }
    appAuthorGroup.sync();
}

KoMainWindow * KoView::shell() const
{
    return dynamic_cast<KoMainWindow *>(window());
}

KXmlGuiWindow * KoView::mainWindow() const
{
    return dynamic_cast<KXmlGuiWindow *>(window());
}

KStatusBar * KoView::statusBar() const
{
    KoMainWindow *mw = shell();
    return mw ? mw->statusBar() : 0;
}

void KoView::slotActionStatusText(const QString &text)
{
    KStatusBar *sb = statusBar();
    if (sb)
        sb->showMessage(text);
}

void KoView::slotClearStatusText()
{
    KStatusBar *sb = statusBar();
    if (sb)
        sb->clearMessage();
}

QToolBar* KoView::viewBar()
{
    if (!d->viewBar) {
        d->viewBar = new QToolBar(statusBar());
        addStatusBarItem(d->viewBar, 0 , true);
    }

    return d->viewBar;
}

QList<QAction*> KoView::createChangeUnitActions()
{
    UnitActionGroup* unitActions = new UnitActionGroup(d->document, this);
    return unitActions->actions();
}

#include <KoView_p.moc>
#include <KoView.moc>
