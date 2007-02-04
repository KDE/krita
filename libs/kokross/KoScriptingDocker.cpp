/*
 * This file is part of the KOffice project
 *
 * Copyright (C) 2006-2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoScriptingDocker.h"

#include <QToolBar>
#include <QBoxLayout>
#include <QTreeView>
#include <QModelIndex>
#include <QHeaderView>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

#include <kross/core/manager.h>
#include <kross/core/model.h>
#include <kross/core/guiclient.h>
//#include <core/actioncollection.h>

/***********************************************************************
 * KoScriptingDockerFactory
 */

class KoScriptingDockerFactory::Private
{
    public:
        QWidget* parent;
        Kross::GUIClient* guiclient;
};

KoScriptingDockerFactory::KoScriptingDockerFactory(QWidget* parent, Kross::GUIClient* guiclient)
    : KoDockFactory()
    , d(new Private())
{
    d->parent = parent;
    d->guiclient = guiclient;
}

KoScriptingDockerFactory::~KoScriptingDockerFactory()
{
    delete d;
}

Kross::GUIClient* KoScriptingDockerFactory::guiClient() const
{
    return d->guiclient;
}

QString KoScriptingDockerFactory::dockId() const
{
    return "Scripting";
}

Qt::DockWidgetArea KoScriptingDockerFactory::defaultDockWidgetArea() const
{
    return Qt::RightDockWidgetArea;
}

QDockWidget* KoScriptingDockerFactory::createDockWidget()
{
    return new KoScriptingDocker(d->parent, d->guiclient);
}

/***********************************************************************
 * KoScriptingDocker
 */

class KoScriptingDocker::Private
{
    public:
        Kross::GUIClient* guiclient;
        Kross::ActionCollectionProxyModel* model;
        QTreeView* view;
};

KoScriptingDocker::KoScriptingDocker(QWidget* parent, Kross::GUIClient* guiclient)
    : QDockWidget(i18n("Scripts"), parent)
    , d(new Private())
{
    d->guiclient = guiclient;

    QWidget* widget = new QWidget(this);
    QBoxLayout* layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    widget->setLayout(layout);

    d->view = new QTreeView(widget);
    d->view->setRootIsDecorated(false);
    d->view->header()->hide();
    d->model = new Kross::ActionCollectionProxyModel(this);
    d->view->setModel(d->model);
    layout->addWidget(d->view, 1);
    d->view->expandAll();

    QToolBar* tb = new QToolBar(widget);
    layout->addWidget(tb);
    tb->setMovable(false);
    //tb->setOrientation(Qt::Vertical);
    //tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tb->addAction(KIcon("player_play"), i18n("Run"), this, SLOT(runScript()) );
    tb->addAction(KIcon("player_stop"), i18n("Stop"), this, SLOT(stopScript()) );

    setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    setWidget(widget);

    connect(d->view, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(runScript()));
}

KoScriptingDocker::~KoScriptingDocker()
{
    delete d;
}

Kross::GUIClient* KoScriptingDocker::guiClient() const
{
    return d->guiclient;
}

void KoScriptingDocker::runScript()
{
    QModelIndex index = d->model->mapToSource( d->view->currentIndex() );
    if( index.isValid() ) {
        Kross::Action* action = Kross::ActionCollectionModel::action(index);
        if( action ) {
            kDebug() << "KoScriptingDocker::runScript execute action=" << action->objectName() << endl;
            action->trigger();
        }
    }
}

void KoScriptingDocker::stopScript()
{
    QModelIndex index = d->model->mapToSource( d->view->currentIndex() );
    if( index.isValid() ) {
        Kross::Action* action = Kross::ActionCollectionModel::action(index);
        if( action ) {
            kDebug() << "KoScriptingDocker::stopScript finalize action=" << action->objectName() << endl;
            action->finalize();
        }
    }
}

#include "KoScriptingDocker.moc"
