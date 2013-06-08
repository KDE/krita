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

#include <Category.h>

#include <QList>
#include <iostream>

void Category::add(Component* c)
{
    m_components.append(c);
}


void Category::processCategoryName(const QString& line)
{

}

Category::Category(Component * parent):m_parent(parent)
{

}

Category::~Category()
{
    qDeleteAll(m_components);
    m_components.clear();
}


void Category::print(int level)
{
    if(!m_components.isEmpty())
    {
        for(int x=0; x < level; ++x) {std::cout << "\t";}

        std::cout << "Category " << qPrintable(name()) << ":\n";
        ++level;
        for (int i = 0; i < m_components.size(); ++i)
            m_components[i]->print(level);
    }
}

Component* Category::child(int index)
{
    if ((index < 0) && (index > m_components.size()))
    {
        return 0;
    }
    else {
        return m_components.at(index);
    }
}

int Category::indexOf(Component* c) const
{
    return m_components.indexOf(c);
}

int Category::row() const
{
    if (m_parent)
    {
        return m_parent->indexOf(const_cast<Category*>(this));
    }
    return 0;
}

int Category::childCount() const
{
    return m_components.size();
}

int Category::columnCount() const
{
    return 1;
}

QVariant Category::data(int column)
{
    return name();
}

