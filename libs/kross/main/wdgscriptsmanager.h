/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef WDGSCRIPTSMANAGER_H
#define WDGSCRIPTSMANAGER_H

#include <main/scriptaction.h>

#include <q3listview.h>
#include <koffice_export.h>

class Scripting;

namespace Kross { namespace Api {

class ScriptGUIClient;
class WdgScriptsManagerPrivate;

/**
@author Cyrille Berger
*/
class KROSS_EXPORT WdgScriptsManager : public QWidget
{
    Q_OBJECT
    public:
        WdgScriptsManager(ScriptGUIClient* scr, QWidget* parent = 0, const char* name = 0);
        ~WdgScriptsManager();
    public slots:
        void slotLoadScript();
        void slotInstallScript();
        void slotUninstallScript();
        void slotExecuteScript();
        void slotUnloadScript();
        void slotGetNewScript();
        void slotSelectionChanged(Q3ListViewItem*);
    private slots:
        void slotFillScriptsList();
        void slotResourceInstalled();
    private:
        WdgScriptsManagerPrivate* d;
        void addItem(ScriptActionCollection* collection);
        Q3ListViewItem* addItem(ScriptAction::Ptr, Q3ListViewItem* parentitem, Q3ListViewItem* afteritem);
};

}}

#endif
