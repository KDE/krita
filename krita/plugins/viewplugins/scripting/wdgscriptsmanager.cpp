/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
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
#include "wdgscriptsmanager.h"

#include <klistview.h>
#include <klocale.h>
#include <ktoolbar.h>

#include "kis_scripts_registry.h"
#include "scripting.h"
#include "kis_script.h"

WdgScriptsManager::WdgScriptsManager(Scripting* scr, QWidget* parent, const char* name, WFlags fl )
    : WdgScriptsManagerBase(parent, name, fl), m_scripting(scr)
{
    scriptsList->addColumn("Name");
    scriptsList->addColumn("Filename");
    
    fillScriptsList();
    
    toolBar->insertButton("fileopen",0, true, i18n("Load script"));
    connect((QObject*)toolBar->getButton(0),SIGNAL(clicked()), this, SLOT(slotLoadScript()));
    toolBar->insertButton("exec",0, true, i18n("Execute script"));
    connect((QObject*)toolBar->getButton(0),SIGNAL(clicked()), this, SLOT(slotExecuteScript()));
    toolBar->insertButton("fileclose",0, true, i18n("Remove script"));
    connect((QObject*)toolBar->getButton(0),SIGNAL(clicked()), this, SLOT(slotRemoveScript()));
}


WdgScriptsManager::~WdgScriptsManager()
{
}

void WdgScriptsManager::fillScriptsList()
{
    scriptsList->clear();
    m_qlviScripts = new QListViewItem(scriptsList, i18n("Scripts"));
    m_qlviScripts->setOpen(true);
//     m_qlviFilters = new QListViewItem(scriptsList, i18n("Filters"));
//     m_qlviTools = new QListViewItem(scriptsList, i18n("Tools"));
    
    KisIDList kl = KisScriptsRegistry::instance()->listKeys();
    for (KisIDList::iterator it = kl.begin(); it !=  kl.end(); ++it) {
        new QListViewItem(m_qlviScripts, (*it).name(), (*it).id());
    }
}

void WdgScriptsManager::slotLoadScript()
{
    if(m_scripting->loadScript(false) != 0)
    {
        fillScriptsList();
    }
}
void WdgScriptsManager::slotExecuteScript()
{
    QListViewItem * current = scriptsList->currentItem ();
    if(current !=0 && current->text(1) != "")
    {
        KisScriptSP s = KisScriptsRegistry::instance()->get(current->text(1));
        if(s!=0) s->execute();
        else kdDebug() << "Script not found" <<endl;
    }
}
void WdgScriptsManager::slotRemoveScript()
{
    QListViewItem * current = scriptsList->currentItem ();
    if(current !=0 && current->text(1) != "")
    {
        KisScriptSP s = KisScriptsRegistry::instance()->remove(current->text(1));
        if(s==0) kdDebug() << "Script not found" <<endl;
        fillScriptsList();
    }
}

#include "wdgscriptsmanager.moc"
