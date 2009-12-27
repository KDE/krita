/***************************************************************************
 * KoScriptingDocker.cpp
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "KoScriptingDocker.h"
#include "KoScriptingModule.h"
#include "KoScriptManager.h"

#include <QToolBar>
#include <QBoxLayout>

#include <klocale.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include <kross/core/action.h>
#include <kross/ui/model.h>
#include <kross/ui/view.h>

/***********************************************************************
 * KoScriptingDockerFactory
 */

KoScriptingDockerFactory::KoScriptingDockerFactory(QWidget *parent, KoScriptingModule *module, Kross::Action *action)
    : KoDockFactory(),
    m_parent(parent),
    m_module(module),
    m_action(action)
{
}

QString KoScriptingDockerFactory::id() const
{
    return m_action ? m_action->name() : "Scripting";
}

KoDockFactory::DockPosition  KoScriptingDockerFactory::defaultDockPosition() const
{
    return DockMinimized;
}

QDockWidget *KoScriptingDockerFactory::createDockWidget()
{
    QDockWidget *dw = 0;
    if (m_action)
        dw = new KoScriptingActionDocker(m_module, m_action, m_parent);
    else
        dw = new KoScriptingDocker(m_parent);
    dw->setObjectName(id());
    return dw;
}

/***********************************************************************
 * KoScriptingDocker
 */

KoScriptingDocker::KoScriptingDocker(QWidget *parent)
    : QDockWidget(i18n("Scripts"), parent)
{
    QWidget *widget = new QWidget(this);
    QBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    widget->setLayout(layout);

    m_view = new Kross::ActionCollectionView(widget);
    m_view->setRootIsDecorated(false);

    //Kross::ActionCollectionModel::Mode modelmode = Kross::ActionCollectionModel::Mode(Kross::ActionCollectionModel::ToolTips);
    //d->model = new Kross::ActionCollectionProxyModel(this, new Kross::ActionCollectionModel(this, 0, modelmode));
    Kross::ActionCollectionProxyModel *model = new Kross::ActionCollectionProxyModel(this);

    m_view->setModel(model);
    layout->addWidget(m_view, 1);
    m_view->expandAll();

    QToolBar *tb = new QToolBar(widget);
    layout->addWidget(tb);
    tb->setMovable(false);
    //tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    KActionCollection *collection = m_view->actionCollection();
    if (QAction *a = collection->action("run")) {
        a = tb->addAction(a->icon(), a->text(), a, SLOT(trigger()));
        a->setEnabled(false);
        m_actions.insert("run", a);
    }
    if (QAction *a = collection->action("stop")) {
        a = tb->addAction(a->icon(), a->text(), a, SLOT(trigger()));
        a->setEnabled(false);
        m_actions.insert("stop", a);
    }

    tb->addAction(KIcon("configure"), i18n("Script Manager"), this, SLOT(slotShowScriptManager()));

    /*
    d->tb->addSeparator();
    QLineEdit *filter = new QLineEdit(tb);
    d->tb->addWidget(filter);
    connect(filter, SIGNAL(textChanged(const QString&)), model, SLOT(setFilterRegExp(const QString&)));
    */

    setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    setWidget(widget);

    connect(m_view, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(slotDoubleClicked()));
    connect(m_view, SIGNAL(enabledChanged(const QString&)), this, SLOT(slotEnabledChanged(const QString&)));
}

void KoScriptingDocker::slotShowScriptManager()
{
    KoScriptManagerDialog *dialog = new KoScriptManagerDialog();
    dialog->exec();
    dialog->delayedDestruct();
}

void KoScriptingDocker::slotEnabledChanged(const QString &actionname)
{
    if (m_actions.contains(actionname))
        if (QAction *a = m_view->actionCollection()->action(actionname))
            m_actions[actionname]->setEnabled(a->isEnabled());
}

void KoScriptingDocker::slotDoubleClicked()
{
    //kDebug(32010)<<"KoScriptingDocker::slotDoubleClicked()";
    m_view->slotRun();
}

/***********************************************************************
 * KoScriptingDocker
 */

KoScriptingActionDocker::KoScriptingActionDocker(KoScriptingModule *module, Kross::Action *action, QWidget *parent)
    : QDockWidget(action->text(), parent),
    m_module(module),
    m_action(action)
{
    kDebug(32010);
    m_action->addObject(this, "KoDocker", Kross::ChildrenInterface::AutoConnectSignals);
    //connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(slotVisibilityChanged(bool)));
}

KoScriptingActionDocker::~KoScriptingActionDocker()
{
    kDebug(32010);
    m_action->finalize();
}

void KoScriptingActionDocker::slotVisibilityChanged(bool visible)
{
    kDebug(32010)<<"visible="<<visible;
    if (visible) {
        if (m_module && m_action->isFinalized()) {
            //KoView *view = m_module->view();
            //KoMainWindow *mainwindow = view ? view->shell() : 0;
            m_action->trigger();
        }
    } else {
        //m_action->finalize();
    }
}

QWidget *KoScriptingActionDocker::widget()
{
    return QDockWidget::widget();
}

void KoScriptingActionDocker::setWidget(QWidget *widget)
{
    QDockWidget::setWidget(widget);
}

#include <KoScriptingDocker.moc>
