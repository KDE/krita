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

#include "KoReportFieldPlugin.h"
#include "KoReportItemField.h"
#include "KoReportDesignerItemField.h"

QString KoReportFieldPlugin::userName()
{
    return i18n("Field");
}

QString KoReportFieldPlugin::iconName()
{
    return "edit-rename";
}

QString KoReportFieldPlugin::entityName()
{
    return "report:field";
}

QObject* KoReportFieldPlugin::createRendererInstance(QDomNode& element)
{
    return new KoReportItemField(element);
}

QObject* KoReportFieldPlugin::createDesignerInstance(QDomNode& element, KoReportDesigner* designer, QGraphicsScene* scene)
{
    return new KoReportDesignerItemField(element, designer, scene);
}

QObject* KoReportFieldPlugin::createDesignerInstance(KoReportDesigner* designer, QGraphicsScene* scene, const QPointF& pos)
{
    return new KoReportDesignerItemField(designer, scene, pos);
}

