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

#include "KoReportTextPlugin.h"
#include "KoReportItemText.h"
#include "KoReportDesignerItemText.h"
#include "KoReportPluginInfo.h"
#include "krscripttext.h"
#include <KoIcon.h>

KoReportTextPlugin::KoReportTextPlugin(QObject *parent, const QVariantList &args) : KoReportPluginInterface(parent)
{
    Q_UNUSED(args)
    KoReportPluginInfo *info = new KoReportPluginInfo();
    info->setClassName("report:text");
    info->setIcon(koIcon("insert-text"));
    info->setName(i18n("Text"));
    info->setPriority(3);
    setInfo(info);
}

KoReportTextPlugin::~KoReportTextPlugin()
{

}

QObject* KoReportTextPlugin::createRendererInstance(QDomNode& element)
{
    return new KoReportItemText(element);
}

QObject* KoReportTextPlugin::createDesignerInstance(QDomNode& element, KoReportDesigner* designer, QGraphicsScene* scene)
{
    return new KoReportDesignerItemText(element, designer, scene);
}

QObject* KoReportTextPlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF& pos)
{
    return new KoReportDesignerItemText(designer, scene, pos);
}

QObject* KoReportTextPlugin::createScriptInstance(KoReportItemBase* item)
{
    KoReportItemText *text = dynamic_cast<KoReportItemText*>(item);
    if (text) {
        return new Scripting::Text(text);
    }
    return 0;
}

