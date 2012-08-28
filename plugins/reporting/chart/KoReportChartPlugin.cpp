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

#include "KoReportChartPlugin.h"
#include "KoReportItemChart.h"
#include "KoReportDesignerItemChart.h"
#include "KoReportPluginInfo.h"
#include "krscriptchart.h"

#include <KoIcon.h>

K_EXPORT_KOREPORT_ITEMPLUGIN(KoReportChartPlugin, chartplugin)

KoReportChartPlugin::KoReportChartPlugin(QObject *parent, const QVariantList &args) : KoReportPluginInterface(parent)
{
    KoReportPluginInfo *info = new KoReportPluginInfo();
    info->setClassName("report:chart");
    info->setIcon(koIcon("office-chart-area"));
    info->setName(i18n("Chart"));
    info->setPriority(10);
    setInfo(info);
}

KoReportChartPlugin::~KoReportChartPlugin()
{

}

QObject* KoReportChartPlugin::createRendererInstance(QDomNode& element)
{
    return new KoReportItemChart(element);
}

QObject* KoReportChartPlugin::createDesignerInstance(QDomNode& element, KoReportDesigner* designer, QGraphicsScene* scene)
{
    return new KoReportDesignerItemChart(element, designer, scene);
}

QObject* KoReportChartPlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF& pos)
{
    return new KoReportDesignerItemChart(designer, scene, pos);
}

QObject* KoReportChartPlugin::createScriptInstance(KoReportItemBase* item)
{
    KoReportItemChart *chart = dynamic_cast<KoReportItemChart*>(item);
    if (chart) {
        return new Scripting::Chart(chart);
    }
    return 0;
}
