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

#include <Filters.h>

#include <QList>

#include <Category.h>

Filters::Filters()
{

}

Filters::~Filters()
{
    qDeleteAll(m_category);
    m_category.clear();
}

void Filters::addCategory(Category* category)
{
    m_category.append(category);
}

void Filters::print()
{
    {
        for (int i = 0; i < m_category.size();i++){
            m_category.at(i)->print(0);
        }
    }
}
