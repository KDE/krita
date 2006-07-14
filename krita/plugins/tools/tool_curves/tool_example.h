/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

/* Initial commit using tool_star. Emanuele Tamponi */

#ifndef TOOL_EXAMPLE_H_
#define TOOL_EXAMPLE_H_

#include <kparts/plugin.h>

class KisView;

/**
 * A module that provides a star tool.
 */
class ToolExample : public KParts::Plugin
{
    Q_OBJECT
public:
    ToolExample(QObject *parent, const char *name, const QStringList &);
    virtual ~ToolExample();

private:

    KisView * m_view;

};

#endif // TOOL_EXAMPLE_H__
