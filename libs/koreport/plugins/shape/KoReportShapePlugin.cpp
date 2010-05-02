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

#include "KoReportShapePlugin.h"
#include "KoReportItemShape.h"
#include "KoReportDesignerItemShape.h"

QString KoReportShapePlugin::userName()
{
    return i18n("Shape");
}

QString KoReportShapePlugin::iconName()
{
    return "shapes"; //TODO new icon
}

QString KoReportShapePlugin::entityName()
{
    return "report:shape";
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

