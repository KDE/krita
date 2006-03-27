/***************************************************************************
 * testwindow.h
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

#ifndef KROSS_TEST_TESTWINDOW_H
#define KROSS_TEST_TESTWINDOW_H

#include "../main/manager.h"
#include "../main/scriptcontainer.h"
#include "../main/scriptguiclient.h"
#include "../api/object.h"

//#include <qobject.h>
#include <qstring.h>

#include <kmainwindow.h>

class QComboBox;
class KTextEdit;

class TestWindow : public KMainWindow
{
        Q_OBJECT
    public:
        TestWindow(const QString& interpretername, const QString& scriptcode);
        virtual ~TestWindow();
    private slots:
        void execute();
    private:
        QString m_interpretername;
        QString m_scriptcode;

        Kross::Api::ScriptContainer::Ptr m_scriptcontainer;
        Kross::Api::ScriptGUIClient* m_scriptextension;

        QComboBox* m_interpretercombo;
        KTextEdit* m_codeedit;
};

#endif
