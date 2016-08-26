/*
 *
 * Copyright (c) 2016 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISOPCUASERVER_H
#define KISOPCUASERVER_H

#include <QThread>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QVariant>

class KisOpcUaServer : public QThread
{
    Q_OBJECT
public:
    static KisOpcUaServer *instance();

    KisOpcUaServer();

    void run();

    void addObject(QObject* object);

public Q_SLOTS:
    bool writeVariable(const QString& id, QVariant value);
 
    QVariant readVariable(const QString& id);
    
private:
    QVector<QObject*> m_objects;
    QMap<QString, QPair<QObject*, int> > m_variableMap;
};

#endif // KISOPCUASERVER_H
