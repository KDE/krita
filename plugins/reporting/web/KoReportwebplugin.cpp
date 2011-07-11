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
 
#include "KoReportwebPlugin.h"
#include "KoReportItemweb.h"
#include "KoReportDesignerItemweb.h"
#include "KoReportPluginInfo.h"
#include "krscriptweb.h"
#include <KIcon>
 
K_EXPORT_KOREPORT_ITEMPLUGIN(KoReportwebPlugin, webplugin)
 
KoReportwebPlugin::KoReportwebPlugin(QObject *parent, const QVariantList &args) : KoReportPluginInterface(parent)
{
    Q_UNUSED(args)
    
    KoReportPluginInfo *info = new KoReportPluginInfo();
    info->setClassName("report:webbrowser");
    info->setIcon(KIcon("marbleicon"));
    info->setName(i18n("web Browser "));
    info->setPriority(40);
    setInfo(info);
}
 
KoReportwebPlugin::~KoReportwebPlugin()
{
 
}
 
QObject* KoReportwebPlugin::createRendererInstance(QDomNode& element)
{
    return new KoReportItemweb(element);
}
 
QObject* KoReportwebPlugin::createDesignerInstance(QDomNode& element, KoReportDesigner* designer, QGraphicsScene* scene)
{
    return new KoReportDesignerItemweb(element, designer, scene);
}
 
QObject* KoReportwebPlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF& pos)
{
    return new KoReportDesignerItemweb(designer, scene, pos);
}
 
QObject* KoReportwebPlugin::createScriptInstance(KoReportItemBase* item)
{
    /*KoReportItemweb *image = dynamic_cast<KoReportItemweb*>(item);
    if (image) {
        return new Scripting::web(image);
    }*/
    return 0;
}
 
