/*
   Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
   Copyright (C) 2011 by Radoslaw Wicik (radoslaw@wicik.pl)

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

#include "KoReportMapsPlugin.h"
#include "KoReportItemMaps.h"
#include "KoReportDesignerItemMaps.h"
#include "KoReportPluginInfo.h"
#include "krscriptmaps.h"
#include <KoIcon.h>
#include <KDebug>


K_EXPORT_KOREPORT_ITEMPLUGIN(KoReportMapsPlugin, mapsplugin)

#define myDebug() kDebug(44021) << "\e[35m=="

KoReportMapsPlugin::KoReportMapsPlugin(QObject *parent, const QVariantList &args) : KoReportPluginInterface(parent)
{
    Q_UNUSED(args)
    
    myDebug() << "\e[35m======\e[0m";
    KoReportPluginInfo *info = new KoReportPluginInfo();
    info->setClassName("report:map");
    info->setIcon(koIcon("report_map_element"));
    info->setName(i18n("Map"));
    info->setPriority(40);
    setInfo(info);
}

KoReportMapsPlugin::~KoReportMapsPlugin()
{
  myDebug() << "\e[35m======\e[0m";
}

QObject* KoReportMapsPlugin::createRendererInstance(QDomNode& element)
{
    myDebug() << "\e[35m======\e[0m";
    return new KoReportItemMaps(element);
}

QObject* KoReportMapsPlugin::createDesignerInstance(QDomNode& element, KoReportDesigner* designer, QGraphicsScene* scene)
{
    myDebug() << "\e[35m======QDomNode, KoReportDesigner, QGraphicsScene\e[0m";
    return new KoReportDesignerItemMaps(element, designer, scene);
}

QObject* KoReportMapsPlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF& pos)
{
    myDebug() << "\e[35m======KoReportDesigner, QGraphicsScene,QPoint\e[0m";
    return new KoReportDesignerItemMaps(designer, scene, pos);
}

QObject* KoReportMapsPlugin::createScriptInstance(KoReportItemBase* item)
{
    myDebug() << "\e[35m======\e[0m";
    /*KoReportItemMaps *image = dynamic_cast<KoReportItemMaps*>(item);
    if (image) {
        return new Scripting::Maps(image);
    }*/
    return 0;
}
