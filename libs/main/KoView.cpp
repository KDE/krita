/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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
#include "KoDockRegistry.h"
#include "KoDocument.h"
#include "KoMainWindow.h"
#include "KoFrame.h"
#include "KoViewAdaptor.h"
#include "KoDockFactory.h"

#include <kactioncollection.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kparts/partmanager.h>
#include <kparts/event.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <QTimer>
#include <QtGui/QDockWidget>
#include <QToolBar>
#include <QApplication>
#include <QListt>


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
        m_manager = 0L;
        m_tempActiveWidget = 0L;
        m_registered = false;
        m_documentDeleted = false;
        m_viewBar = 0L;
    }
    ~KoViewPrivate() {
    }

    QPointer<KoDocument> m_doc; // our KoDocument
    QPointer<KParts::PartManager> m_manager;
    QWidget *m_tempActiveWidget;
    bool m_registered;  // are we registered at the part manager?
    bool m_documentDeleted; // true when m_doc gets deleted [can't use m_doc==0
    // since this only happens in ~QObject, and views
    // get deleted by ~KoDocument].
    QTimer *m_scrollTimer;


    // Hmm sorry for polluting the private class with such a big inner class.
    // At the beginning it was a little struct :)
    class StatusBarItem
    {
    public:
        StatusBarItem() // for QValueList
                : m_widget(0), m_visible(false) {}
        StatusBarItem(QWidget * widget, int stretch, bool permanent)
                : m_widget(widget), m_stretch(stretch), m_permanent(permanent), m_visible(false) {}

        QWidget * widget() const {
            return m_widget;
        }

        void ensureItemShown(KStatusBar * sb) {
            Q_ASSERT(m_widget);
            if (!m_visible) {
                if (m_permanent)
                    sb->addPermanentWidget(m_widget, m_stretch);
                else
                    sb->addWidget(m_widget, m_stretch);

                m_visible = true;
                m_widget->show();
            }
        }
        void ensureItemHidden(KStatusBar * sb) {
            if (m_visible) {
                sb->removeWidget(m_widget);
                m_visible = false;
                m_widget->hide();
            }
        }
    private:
        QWidget * m_widget;
        int m_stretch;
        bool m_permanent;
        bool m_visible;  // true when the item has been added to the statusbar
    };
    QList<StatusBarItem> m_statusBarItems; // Our statusbar items
    bool m_inOperation; //in the middle of an operation (no screen refreshing)?
    QToolBar* m_viewBar;
};

KoView::KoView(KoDocument *document, QWidget *parent)
        : QWidget(parent)
        , d(new KoViewPrivate)
{
    Q_ASSERT(document);

    setObjectName(newObjectName());

    new KoViewAdaptor(this);
    QDBusConnection::sessionBus().registerObject('/' + objectName(), this);

    //kDebug(30003) <<"KoView::KoView" << this;
    d->m_doc = document;
    KParts::PartBase::setPartObject(this);

    setFocusPolicy(Qt::StrongFocus);

    setupGlobalActions();

    KStatusBar * sb = statusBar();
    if (sb) { // No statusbar in e.g. konqueror
        //coll->setHighlightingEnabled( true );
#ifdef __GNUC__
#warning portKDE4
#endif
#if 0
        connect(coll, SIGNAL(actionStatusText(const QString &)),
                this, SLOT(slotActionStatusText(const QString &)));
        connect(coll, SIGNAL(clearStatusText()),
                this, SLOT(slotClearStatusText()));
#endif

        connect(d->m_doc, SIGNAL(statusBarMessage(const QString&)),
                this, SLOT(slotActionStatusText(const QString&)));
        connect(d->m_doc, SIGNAL(clearStatusBarMessage()),
                this, SLOT(slotClearStatusText()));
    }
    d->m_doc->setCurrent();

    d->m_scrollTimer = new QTimer(this);
    connect(d->m_scrollTimer, SIGNAL(timeout()), this, SLOT(slotAutoScroll()));

    // add all plugins.
    foreach(const QString & docker, KoDockRegistry::instance()->keys()) {
        KoDockFactory *factory = KoDockRegistry::instance()->value(docker);
        createDockWidget(factory);
    }

    actionCollection()->addAssociatedWidget(this);
    foreach(QAction* action, actionCollection()->actions())
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

KoView::~KoView()
{
    kDebug(30003) << "KoView::~KoView" << this;
    delete d->m_scrollTimer;
//   delete d->m_dcopObject;
    if (!d->m_documentDeleted) {
        if (koDocument() && !koDocument()->isSingleViewMode()) {
            if (d->m_manager && d->m_registered)   // if we aren't registered we mustn't unregister :)
                d->m_manager->removePart(koDocument());
            d->m_doc->removeView(this);
            d->m_doc->setCurrent(false);
        }
    }
    delete d;
}

KoDocument *KoView::koDocument() const
{
    return d->m_doc;
}

void KoView::setDocumentDeleted()
{
    d->m_documentDeleted = true;
}

bool KoView::documentDeleted() const
{
    return d->m_documentDeleted;
}

void KoView::setPartManager(KParts::PartManager *manager)
{
    d->m_manager = manager;
    if (!koDocument()->isSingleViewMode() &&
            !manager->parts().contains(koDocument())) {  // is there another view registered?
        d->m_registered = true; // no, so we have to register now and ungregister again in the DTOR
        manager->addPart(koDocument(), false);
    } else
        d->m_registered = false;  // There is already another view registered for that part...
}

KParts::PartManager *KoView::partManager() const
{
    return d->m_manager;
}

QAction *KoView::action(const QDomElement &element) const
{
    static const QString &attrName = KGlobal::staticQString("name");
    QString name = element.attribute(attrName);

    QAction *act = KXMLGUIClient::action(name.toUtf8());

    if (!act)
        act = d->m_doc->KXMLGUIClient::action(name.toUtf8());

    // last resort, try to get action from the main window if there is one
    if (!act && shell())
        act = shell()->actionCollection()->action(name);

    return act;
}

KoDocument *KoView::hitTest(const QPoint &viewPos)
{
    Q_UNUSED(viewPos);
    return koDocument(); // we no longer have child documents
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
    foreach(KoViewPrivate::StatusBarItem sbItem, d->m_statusBarItems) {
        if (show)
            sbItem.ensureItemShown(sb);
        else
            sbItem.ensureItemHidden(sb);
    }
}


void KoView::addStatusBarItem(QWidget * widget, int stretch, bool permanent)
{
    KoViewPrivate::StatusBarItem item(widget, stretch, permanent);
    d->m_statusBarItems.append(item);
    KoViewPrivate::StatusBarItem item = d->m_statusBarItems.last();
    KStatusBar * sb = statusBar();
    if (sb) {
        item.ensureItemShown(sb);
    }
}

void KoView::removeStatusBarItem(QWidget * widget)
{
    KStatusBar * sb = statusBar();
    
    foreach(KoViewPrivate::StatusBarItem sbItem, d->m_statusBarItems) {
        if (sbItem.widget() == widget) {
            if (sb) {
                sbItem.ensureItemHidden(sb);
            }
            m_statusBarItems.removeOne(sb);
            break;
        }
    }
}

void KoView::enableAutoScroll()
{
    d->m_scrollTimer->start(50);
}

void KoView::disableAutoScroll()
{
    d->m_scrollTimer->stop();
}

void KoView::paintEverything(QPainter &painter, const QRect &rect)
{
    koDocument()->paintEverything(painter, rect, this);
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

void KoView::setupGlobalActions()
{
    actionNewView  = new KAction(KIcon("window-new"), i18n("&New View"), this);
    actionCollection()->addAction("view_newview", actionNewView);
    connect(actionNewView, SIGNAL(triggered(bool)), this, SLOT(newView()));
}

void KoView::newView()
{
    Q_ASSERT((d != 0L && d->m_doc));

    KoDocument *thisDocument = d->m_doc;
    KoMainWindow *shell = new KoMainWindow(thisDocument->componentData());
    shell->setRootDocument(thisDocument);
    shell->show();
}

KoDockerManager * KoView::dockerManager() const
{
    KoMainWindow *mw = shell();
    return mw ? mw->dockerManager() : 0;
}

void KoView::setDockerManager(KoDockerManager *dm)
{
    KoMainWindow *mw = shell();

    if(mw)
        mw->setDockerManager(dm);
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
    return mw ? mw->statusBar() : 0L;
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

// DCOPObject *KoView::dcopObject()
// {
//     if ( !d->m_dcopObject )
//         d->m_dcopObject = new KoViewIface( this );
//     return d->m_dcopObject;
// }

QDockWidget *KoView::createDockWidget(KoDockFactory* factory)
{
    return shell() ? shell()->createDockWidget(factory) : 0;
}

void KoView::removeDockWidget(QDockWidget *dock)
{
    if (shell())
        shell()->removeDockWidget(dock);
}

void KoView::restoreDockWidget(QDockWidget *dock)
{
    if (shell())
        shell()->restoreDockWidget(dock);
}

QToolBar* KoView::viewBar()
{
    if (!d->m_viewBar) {
        d->m_viewBar = new QToolBar(statusBar());
        addStatusBarItem(d->m_viewBar, 0 , true);
    }

    return d->m_viewBar;
}

QList<QAction*> KoView::createChangeUnitActions()
{
    QList<QAction*> answer;
    answer.append(new UnitChangeAction(KoUnit::Millimeter, this, d->m_doc));
    answer.append(new UnitChangeAction(KoUnit::Centimeter, this, d->m_doc));
    answer.append(new UnitChangeAction(KoUnit::Decimeter, this, d->m_doc));
    answer.append(new UnitChangeAction(KoUnit::Inch, this, d->m_doc));
    answer.append(new UnitChangeAction(KoUnit::Pica, this, d->m_doc));
    answer.append(new UnitChangeAction(KoUnit::Cicero, this, d->m_doc));
    answer.append(new UnitChangeAction(KoUnit::Point, this, d->m_doc));

    return answer;
}

#include "KoView_p.moc"
#include "KoView.moc"
