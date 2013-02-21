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

#ifndef KOREPORTLABELPLUGIN_H
#define KOREPORTLABELPLUGIN_H

#include <KoReportPluginInterface.h>

class KoReportLabelPlugin : public KoReportPluginInterface
{
    public:
    explicit KoReportLabelPlugin(QObject *parent, const QVariantList &args = QVariantList());
    
    virtual ~KoReportLabelPlugin();

    virtual QObject* createRendererInstance(QDomNode&);
    virtual QObject* createDesignerInstance(KoReportDesigner* , QGraphicsScene* scene, const QPointF&);
    virtual QObject* createDesignerInstance(QDomNode & element, KoReportDesigner *, QGraphicsScene * scene);
    virtual QObject* createScriptInstance(KoReportItemBase* item);
};

#endif // KOREPORTLABELPLUGIN_H
