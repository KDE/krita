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


#ifndef COMMAND_H
#define COMMAND_H

#include <Component.h>

#include <QList>
class QString;
class QStringList;

class Parameter;

class Command : public Component
{
public:
    Command(Component * parent = 0);
    virtual ~Command();

    QString m_command;
    QString m_commandPreview;
    QList<Parameter*> m_parameters;

    virtual void add(Component* c);
    virtual Component* child(int index);
    virtual Component* parent();
    virtual int row() const;
    virtual int childCount() const;
    virtual int columnCount() const;
    virtual QVariant data(int column);

    void print(int level);

    void processCommandName(const QString &line);
    void processParameter(const QString &line);
private:
    Component * m_parent;
    QStringList breakIntoTokens(const QString &line);
};

#endif
