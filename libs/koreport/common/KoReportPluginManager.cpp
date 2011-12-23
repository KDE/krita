/*
   KoReport Library
   Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoReportPluginManager.h"
#include "KoReportPluginManagerPrivate.h"
#include "KoReportPluginInfo.h"

#include <KIcon>
#include <KService>
#include <KServiceTypeTrader>
#include <KIconLoader>
#include <KToggleAction>

//Include the static items
#include "../items/label/KoReportLabelPlugin.h"
#include "../items/check/KoReportCheckPlugin.h"
#include "../items/field/KoReportFieldPlugin.h"
#include "../items/image/KoReportImagePlugin.h"
#include "../items/text/KoReportTextPlugin.h"

KoReportPluginManager* KoReportPluginManager::self()
{
    K_GLOBAL_STATIC(KoReportPluginManager, instance) // only instantiated when self() is called
    return instance;
}

KoReportPluginManager::KoReportPluginManager() : d(new KoReportPluginManagerPrivate())
{
    KIconLoader::global()->addAppDir("calligra");
}

KoReportPluginManager::~KoReportPluginManager()
{
    delete d;
}

KoReportPluginInterface* KoReportPluginManager::plugin(const QString& p) const
{
    if (d->m_plugins.contains(p)) {
        return d->m_plugins[p];
    }
    return 0;
}

QList<QAction*> KoReportPluginManager::actions()
{
    QList<QAction*> actList;

    KoReportDesigner designer(0);
    const QMap<QString, KoReportPluginInterface*> plugins = d->m_plugins;

    foreach(KoReportPluginInterface* plugin, plugins) {
        KoReportPluginInfo *info = plugin->info();
        if (info) {
            KToggleAction *act = new KToggleAction(KIcon(info->icon()), info->name(), this);
            act->setObjectName(info->className());

            //Store the order priority in the user data field
            act->setData(info->priority());
            actList << act;
        }
    }
    
    return actList;
}


//===============================Private========================================

KoReportPluginManagerPrivate::KoReportPluginManagerPrivate()
{
    //Create the static items here

    KoReportPluginInterface *plugin = 0;

    plugin = new KoReportLabelPlugin(this);
    m_plugins.insert(plugin->info()->className(), plugin);
    
    plugin = new KoReportCheckPlugin(this);
    m_plugins.insert(plugin->info()->className(), plugin);

    plugin = new KoReportFieldPlugin(this);
    m_plugins.insert(plugin->info()->className(), plugin);

    plugin = new KoReportImagePlugin(this);
    m_plugins.insert(plugin->info()->className(), plugin);

    plugin = new KoReportTextPlugin(this);
    m_plugins.insert(plugin->info()->className(), plugin);

    //And then load the plugins
    loadPlugins();
}

void KoReportPluginManagerPrivate::loadPlugins()
{
    kDebug() << "Load all plugins";
    KService::List offers = KServiceTypeTrader::self()->query("KoReport/ItemPlugin");

    KService::List::const_iterator iter;
    for(iter = offers.constBegin(); iter < offers.constEnd(); ++iter)
    {
       QString error;
       KService::Ptr service = *iter;

        KPluginFactory *factory = KPluginLoader(service->library()).factory();

        if (!factory)
        {
            kDebug() << "KPluginFactory could not load the plugin:" << service->library();
            continue;
        }

       KoReportPluginInterface *plugin = factory->create<KoReportPluginInterface>(this);

       if (plugin) {
           kDebug() << "Load plugin:" << service->name();

           plugin->info()->setPriority(plugin->info()->priority() + 10); //Ensure plugins always have a higher prioroty than built-in types
           m_plugins.insert(plugin->info()->className(), plugin);
       } else {
           kDebug() << error;
       }
    }

}

KoReportPluginManagerPrivate::~KoReportPluginManagerPrivate()
{

}

#include "moc_KoReportPluginManager.cpp"
#include "moc_KoReportPluginManagerPrivate.cpp"
