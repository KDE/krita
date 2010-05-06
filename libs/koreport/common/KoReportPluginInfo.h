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

#ifndef KOREPORTPLUGININFO_H
#define KOREPORTPLUGININFO_H

#include <QString>
#include "koreport_export.h"

class KOREPORT_EXPORT KoReportPluginInfo
{
    public:
        KoReportPluginInfo();
        ~KoReportPluginInfo();

        QString entityName() const;
        QString iconName() const;
        QString userName() const;
        int priority() const;
        
        void setEntityName(const QString&);
        void setIconName(const QString&);
        void setUserName(const QString&);
        void setPriority(int);
    private:
        QString m_entityName;
        QString m_iconName;
        QString m_userName;
        int m_priority;
    
};

#endif // KOREPORTPLUGININFO_H
