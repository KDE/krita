/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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


#ifndef COMPONENT_H
#define COMPONENT_H

#include <QString>
#include <QVariant>

class Component
{
public:
    Component(){};
    virtual ~Component(){};
    virtual void add(Component *c) = 0;
    virtual Component * child(int index) = 0;
    virtual Component * parent() = 0;
    virtual int row() const = 0;

    virtual int childCount() const = 0;
    virtual int columnCount() const = 0;

    virtual int indexOf(Component *c) const { Q_UNUSED(c); return 0; }

    virtual void setName(const QString &name) {  m_name = name; }
    virtual QString name() const { return m_name; }

    virtual void print(int level = 0) { Q_UNUSED(level); }

    virtual QVariant data(int column){ return QVariant();}

private:
    QString m_name;

};

#endif
