/***************************************************************************
 * testaction.h
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

#ifndef KROSS_TEST_TESTACTION_H
#define KROSS_TEST_TESTACTION_H

#include "../main/scriptcontainer.h"

#include <qobject.h>
#include <qwidget.h>
#include <qstring.h>

#include <kaction.h>
#include <kactioncollection.h>

class TestAction : public QWidget
{
        Q_OBJECT
    public:
        TestAction(Kross::Api::ScriptContainer::Ptr scriptcontainer);
        ~TestAction();

    private slots:
        void activatedAction1();
        void activatedAction2();

    private:
        KAction* m_action1;
        KAction* m_action2;
        KActionCollection* m_actioncollection;
        KActionPtrList m_actionlist;
};

#endif
