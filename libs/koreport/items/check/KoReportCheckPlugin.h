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

#ifndef KOREPORTCHECKPLUGIN_H
#define KOREPORTCHECKPLUGIN_H

#include <KoReportPluginInterface.h>


class KoReportCheckPlugin : public KoReportPluginInterface
{
    public:
        explicit KoReportCheckPlugin(QObject *parent, const QVariantList &args = QVariantList());
        virtual ~KoReportCheckPlugin();

        virtual QObject* createRendererInstance(QDomNode& element);
        virtual QObject* createDesignerInstance(QDomNode& element, KoReportDesigner* , QGraphicsScene* scene);
        virtual QObject* createDesignerInstance(KoReportDesigner* , QGraphicsScene* scene, const QPointF& pos);
        virtual QObject* createScriptInstance(KoReportItemBase* item);
};

#endif // KOREPORTCHECKPLUGIN_H
