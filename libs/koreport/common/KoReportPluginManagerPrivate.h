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

#ifndef KOREPORTPLUGINMANAGERP_H
#define KOREPORTPLUGINMANAGERP_H

#include <QObject>
#include <QMap>

class KoReportPluginInterface;

class KoReportPluginManagerPrivate : public QObject {
    Q_OBJECT
    public:
        KoReportPluginManagerPrivate();
        ~KoReportPluginManagerPrivate();
        void loadPlugins();
        //!Map of name -> plugin instances
        QMap<QString, KoReportPluginInterface*> m_plugins;
};


#endif
