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
#include <QModelIndex>

#include <QRect>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

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
    m_model = new Kross::ActionCollectionProxyModel(this);
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
}

ScriptingDocker::~ScriptingDocker()
{
}

void ScriptingDocker::runScript()
{
    QModelIndex index = m_model->mapToSource( m_view->currentIndex() );
    if( index.isValid() ) {
        Kross::Action* action = static_cast< Kross::Action* >( index.internalPointer() );
        action->trigger(); // execute the script
    }
}

void ScriptingDocker::stopScript()
{
    QModelIndex index = m_model->mapToSource( m_view->currentIndex() );
    if( index.isValid() ) {
        Kross::Action* action = static_cast< Kross::Action* >( index.internalPointer() );
        action->finalize();
    }
}

#include "scriptingdocker.moc"
