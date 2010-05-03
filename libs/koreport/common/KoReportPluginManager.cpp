/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

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

#include <kicon.h>
#include <QAction>

//!Temp load all plugins here until the loader is created
#include "../plugins/barcode/KoReportBarcodePlugin.h"
#include "../plugins/chart/KoReportChartPlugin.h"
#include "../plugins/check/KoReportCheckPlugin.h"
#include "../plugins/field/KoReportFieldPlugin.h"
#include "../plugins/image/KoReportImagePlugin.h"
#include "../plugins/label/KoReportLabelPlugin.h"
#include "../plugins/shape/KoReportShapePlugin.h"
#include "../plugins/text/KoReportTextPlugin.h"


KoReportPluginManager& KoReportPluginManager::self()
{
  static KoReportPluginManager instance; // only instantiated when self() is called
  return instance;
}

KoReportPluginManager::KoReportPluginManager()
{
    d = new Private();
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
    QAction *act;

    KoReportDesigner designer(0);
    const QMap<QString, KoReportPluginInterface*> plugins = d->m_plugins;

    foreach(KoReportPluginInterface* plugin, plugins) {
        act = new QAction(KIcon(plugin->iconName()), plugin->userName(), 0);
        act->setObjectName(plugin->entityName());
        actList << act;
    }

    return actList;

}


//===============================Private========================================

KoReportPluginManager::Private::Private()
{
    //!Temp - Add Plugins Here

    m_plugins.insert("report:barcode", new KoReportBarcodePlugin());
    m_plugins.insert("report:chart", new KoReportChartPlugin());
    m_plugins.insert("report:check", new KoReportCheckPlugin());
    m_plugins.insert("report:field", new KoReportFieldPlugin());
    m_plugins.insert("report:image", new KoReportImagePlugin());
    m_plugins.insert("report:label", new KoReportLabelPlugin());
    m_plugins.insert("report:shape", new KoReportShapePlugin());
    m_plugins.insert("report:text", new KoReportTextPlugin());

    //!End Add Plugins
}

KoReportPluginManager::Private::~Private()
{

}
