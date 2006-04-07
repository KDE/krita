/***************************************************************************
 * testaction.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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
 ***************************************************************************/

#include "testaction.h"

TestAction::TestAction(Kross::Api::ScriptContainer::Ptr scriptcontainer)
    : QWidget()
{
    m_actioncollection = new KActionCollection(this, this);

    m_action1 = new KAction("Action1_Text", 0, this, SLOT(activatedAction1()), m_actioncollection, "Action1");
    m_actionlist.append(m_action1);
    scriptcontainer->addKAction(m_action1);

    m_action2 = new KAction("Action2_Text", 0, this, SLOT(activatedAction2()), m_actioncollection, "Action2");
    m_actionlist.append(m_action2);
    scriptcontainer->addKAction(m_action2);
}

TestAction::~TestAction()
{
}

void TestAction::activatedAction1()
{
    krossdebug("TestAction::activatedAction1()");
}

void TestAction::activatedAction2()
{
    krossdebug("TestAction::activatedAction2()");
}

