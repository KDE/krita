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

#include "KoReportLabelPlugin.h"
#include "KoReportItemLabel.h"
#include "KoReportDesignerItemLabel.h"
#include "KoReportDesigner.h"
#include "KoReportPluginInfo.h"

K_EXPORT_KOREPORT_ITEMPLUGIN(KoReportLabelPlugin, labelplugin)

KoReportLabelPlugin::KoReportLabelPlugin(QObject *parent, const QVariantList &args) : KoReportPluginInterface(parent, args)
{
    KoReportPluginInfo *info = new KoReportPluginInfo();
    info->setEntityName("report:label");
    info->setIconName("label");
    info->setUserName(i18n("Label"));
    info->setPriority(1);

    setInfo(info);
}

KoReportLabelPlugin::~KoReportLabelPlugin()
{

}

QObject* KoReportLabelPlugin::createRendererInstance(QDomNode &elem)
{
    return new KoReportItemLabel(elem);
}

QObject* KoReportLabelPlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF &pos)
{
    return new KoReportDesignerItemLabel(designer, scene, pos);
}

QObject* KoReportLabelPlugin::createDesignerInstance(QDomNode & element, KoReportDesigner *designer, QGraphicsScene * scene)
{
    return new KoReportDesignerItemLabel(element, designer, scene);
}


#include "moc_KoReportLabelPlugin.cpp"