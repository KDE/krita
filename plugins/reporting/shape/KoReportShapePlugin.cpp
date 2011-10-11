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

#include "KoReportShapePlugin.h"
#include "KoReportItemShape.h"
#include "KoReportDesignerItemShape.h"
#include "KoReportPluginInfo.h"

#include <KIcon>

K_EXPORT_KOREPORT_ITEMPLUGIN(KoReportShapePlugin, shapeplugin)

KoReportShapePlugin::KoReportShapePlugin(QObject *parent, const QVariantList &/*args*/) : KoReportPluginInterface(parent)
{
    KGlobal::locale()->insertCatalog("ReportingShapePlugin");
    KoReportPluginInfo *info = new KoReportPluginInfo();
    info->setClassName("report:shape");
    info->setIcon(KIcon("report_shape_element"));
    info->setName(i18n("Shape"));
    info->setPriority(0);
    setInfo(info);
}

KoReportShapePlugin::~KoReportShapePlugin()
{

}

QObject* KoReportShapePlugin::createRendererInstance(QDomNode& element)
{
    return new KoReportItemShape(element);
}

QObject* KoReportShapePlugin::createDesignerInstance(QDomNode& element, KoReportDesigner* designer, QGraphicsScene* scene)
{
    return new KoReportDesignerItemShape(element, designer, scene);
}

QObject* KoReportShapePlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF& pos)
{
    return new KoReportDesignerItemShape(designer, scene, pos);
}

QObject* KoReportShapePlugin::createScriptInstance(KoReportItemBase* /*item*/)
{
    return 0;
}
