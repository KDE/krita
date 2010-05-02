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

#include "KoReportCheckPlugin.h"
#include "KoReportItemCheck.h"
#include "KoReportDesignerItemCheck.h"

QString KoReportCheckPlugin::userName()
{
    return i18n("Check");
}

QString KoReportCheckPlugin::iconName()
{
    return "checkbox";
}

QString KoReportCheckPlugin::entityName()
{
    return "report:check";
}

QObject* KoReportCheckPlugin::createRendererInstance(QDomNode& element)
{
    return new KoReportItemCheck(element);
}

QObject* KoReportCheckPlugin::createDesignerInstance(QDomNode& element, KoReportDesigner* designer, QGraphicsScene* scene)
{
    return new KoReportDesignerItemCheck(element, designer, scene);
}

QObject* KoReportCheckPlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF& pos)
{
    return new KoReportDesignerItemCheck(designer, scene, pos);
}

