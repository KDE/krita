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

#ifndef KOREPORTPLUGININTERFACE_H
#define KOREPORTPLUGININTERFACE_H

#include <QObject>
#include <KoReportDesigner.h>
#include <QGraphicsScene>
#include <KLocalizedString>
#include "koreport_export.h"
#include <KPluginFactory>

class KService;
class KoReportPluginInfo;

class KOREPORT_EXPORT KoReportPluginInterface : public QObject
{
    Q_OBJECT
    public:
        explicit KoReportPluginInterface(QObject *parent = 0, const QVariantList &args = QVariantList());
    
        virtual ~KoReportPluginInterface();
        
        virtual QObject* createDesignerInstance(KoReportDesigner *, QGraphicsScene * scene, const QPointF &pos) = 0;
        virtual QObject* createDesignerInstance(QDomNode & element, KoReportDesigner *, QGraphicsScene * scene) = 0;
        virtual QObject* createRendererInstance(QDomNode & element) = 0;
        virtual QObject* createScriptInstance(KoReportItemBase* item) = 0;

        void setInfo(KoReportPluginInfo *);
        KoReportPluginInfo* info() const;

    private:
        KoReportPluginInfo *m_pluginInfo;
        
};

//! Implementation of driver's static version information and plugin entry point.

    
#define K_EXPORT_KOREPORT_ITEMPLUGIN( class_name, internal_name ) \
    K_PLUGIN_FACTORY(factory, registerPlugin<class_name>();) \
    K_EXPORT_PLUGIN(factory("koreport_" # internal_name)) \
    K_EXPORT_PLUGIN_VERSION(KDE_MAKE_VERSION(0, 0, 1))

#endif // KOREPORTPLUGININTERFACE_H
