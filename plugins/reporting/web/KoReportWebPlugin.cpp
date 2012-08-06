/* This file is part of the KDE project
   Copyright Shreya Pandit <shreya@shreyapandit.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoReportWebPlugin.h"
#include "KoReportDesignerItemWeb.h"
#include "KoReportPluginInfo.h"
#include <KoIcon.h>

K_EXPORT_KOREPORT_ITEMPLUGIN(KoReportWebPlugin, webplugin)

KoReportWebPlugin::KoReportWebPlugin(QObject *parent, const QVariantList &args)
    : KoReportPluginInterface(parent)
{
    Q_UNUSED(args)

    KoReportPluginInfo *info = new KoReportPluginInfo();
    info->setClassName("report:web");
    info->setName(i18n("Web"));
    info->setIcon(koIcon("report_web_element"));
    info->setPriority(40);
    setInfo(info);
}

KoReportWebPlugin::~KoReportWebPlugin()
{
}

QObject *KoReportWebPlugin::createRendererInstance(QDomNode &element)
{
    return new KoReportItemWeb(element);
}

QObject *KoReportWebPlugin::createDesignerInstance(QDomNode &element, KoReportDesigner *designer,
                                                   QGraphicsScene *scene)
{
    return new KoReportDesignerItemWeb(element, designer, scene);
}

QObject *KoReportWebPlugin::createDesignerInstance(KoReportDesigner *designer,
                                                   QGraphicsScene *scene,const QPointF &pos)
{
    return new KoReportDesignerItemWeb(designer, scene, pos);
}

QObject *KoReportWebPlugin::createScriptInstance(KoReportItemBase *item)
{
    Q_UNUSED(item);
//   KoReportItemweb *image = dynamic_cast<KoReportItemweb*>(item);
    // if (image) {
    //   return new Scripting::Web(image);
    return 0;
}
