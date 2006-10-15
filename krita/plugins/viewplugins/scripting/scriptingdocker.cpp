/*
 * This file is part of the KDE project
 *
 * Copyright (C) 2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "scriptingdocker.h"

#include <QToolBar>
#include <QBoxLayout>
#include <QListView>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

#include <core/manager.h>
#include <core/model.h>
#include <core/guiclient.h>

ScriptingDocker::ScriptingDocker(QWidget* parent, Kross::GUIClient* guiclient)
    : QWidget(parent)
    , m_guiclient(guiclient)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    m_view = new QListView(this);
    m_model = new Kross::ActionMenuModel(this, Kross::Manager::self().actionMenu());
    m_view->setModel(m_model);
    layout->addWidget(m_view, 1);

    QToolBar* tb = new QToolBar(this);
    layout->addWidget(tb);
    tb->setMovable(false);
    //tb->setOrientation(Qt::Vertical);
    //tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tb->addAction(KIcon("player_play"), i18n("Run"), this, SLOT(runScript()) );
    tb->addAction(KIcon("player_stop"), i18n("Stop"), this, SLOT(stopScript()) );

    connect(m_view, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(runScript()));
    connect(&Kross::Manager::self(), SIGNAL(configChanged()), this, SLOT(dataChanged()));
}

ScriptingDocker::~ScriptingDocker()
{
}

void ScriptingDocker::runScript()
{
    if( m_view->currentIndex().isValid() ) {
        Kross::Action* action = dynamic_cast< Kross::Action* >( static_cast< KAction* >( m_view->currentIndex().internalPointer() ) );
        if( action )
            m_guiclient->executeAction(action);
    }
}

void ScriptingDocker::stopScript()
{
    if( m_view->currentIndex().isValid() ) {
        Kross::Action* action = dynamic_cast< Kross::Action* >( static_cast< KAction* >( m_view->currentIndex().internalPointer() ) );
        if( action )
            action->finalize();
    }
}

void ScriptingDocker::dataChanged() const
{
    kDebug() << "GUIManagerView::dataChanged() ----------------------------- !!!!!!!!!!!!!!!!!!" << endl;
    //m_view->dataChanged( QModelIndex(), QModelIndex() );
    m_view->update();
    //m_view->executeDelayedItemsLayout();
    m_view->setModel(m_model);
}

#include "scriptingdocker.moc"
