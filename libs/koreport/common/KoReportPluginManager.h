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

#ifndef KOREPORTPLUGINMANAGER_H
#define KOREPORTPLUGINMANAGER_H

#include <QObject>
#include <KoReportPluginInterface.h>

class KoReportPluginManager : public QObject
{
    Q_OBJECT
    public:
        static KoReportPluginManager &self();

        KoReportPluginInterface* plugin(const QString& p) const;
        QList<QAction*> actions();
        
    private:
        class Private {
            public:
                Private();
                ~Private();

                //!Map of name -> plugin instances
                QMap<QString, KoReportPluginInterface*> m_plugins;
        };

        
        Private *d;

        KoReportPluginManager();
        ~KoReportPluginManager();
        KoReportPluginManager(const KoReportPluginManager &);
        KoReportPluginManager & operator=(const KoReportPluginManager &); 
};

#endif // KOREPORTPLUGINMANAGER_H
