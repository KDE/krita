/*
 * actionsearch.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
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

#include "SearchPlugin.h"

#include <QDebug>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <kis_action.h>
#include <kis_action_manager.h>
#include <QAction>
#include <QWidgetAction>

#include "SearchWidget.h"

K_PLUGIN_FACTORY_WITH_JSON(SearchPluginFactory, "kritasearch.json", registerPlugin<SearchPluginFactory>();)

SearchPlugin::SearchPlugin(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    m_actionSearchLine = new SearchWidget(viewManager()->actionCollection(), 0);
    m_searchAction = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("searchaction", m_searchAction);
    viewManager()->actionCollection()->addAction("searchaction", m_searchAction);
    m_searchAction->setDefaultWidget(m_actionSearchLine);
}


#include "SearchPlugin.moc"
