/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef SELECTION_TOOLS_H_
#define SELECTION_TOOLS_H_

#include <kparts/plugin.h>

/**
 * A module wrapper around Krita's selection tools.
 * Despite the fact that new tools are created for every new view,
 * it is not possible to make tools standard parts of the type of the
 * imagesize plugin, because we need to create a new set of tools for every
 * pointer device (mouse, stylus, eraser, puck, etc.). So this plugin is
 * a module which is loaded once into Krita. For every tool there is a factory
 * class that is registered with the tool registry, and that is used to create
 * new instances of the tools.
 */
class SelectionTools : public KParts::Plugin
{
    Q_OBJECT
public:
    SelectionTools(QObject *parent, const char *name, const QStringList &);
    virtual ~SelectionTools();

};

#endif // SELECTION_TOOLS_H_
