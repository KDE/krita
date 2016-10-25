/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Selection.h"

struct Selection::Private {
    Private() {}
};

Selection::Selection(QObject *parent) 
    : QObject(parent)
    , d(new Private)
{
}

Selection::~Selection() 
{
    delete d;
}

int Selection::width() const
{
    return 0;
}

void Selection::setWidth(int value)
{
}


int Selection::height() const
{
    return 0;
}

void Selection::setHeight(int value)
{
}


int Selection::x() const
{
    return 0;
}

void Selection::setX(int value)
{
}


int Selection::y() const
{
    return 0;
}

void Selection::setY(int value)
{
}


QString Selection::type() const
{
    return QString();
}

void Selection::setType(QString value)
{
}

void Selection::clear()
{
}

void Selection::contract(int value)
{
}

Selection* Selection::copy(int x, int y, int w, int h)
{
    return 0;
}

void Selection::cut(Node* node)
{
}

void Selection::deselect()
{
}

void Selection::expand(int value)
{
}

void Selection::feather(int value)
{
}

void Selection::fill(Node* node)
{
}

void Selection::grow(int value)
{
}

void Selection::invert()
{
}

void Selection::resize(int w, int h)
{
}

void Selection::rotate(int degrees)
{
}

void Selection::select(int x, int y, int w, int h, int value)
{
}

void Selection::selectAll(Node *node)
{
}



