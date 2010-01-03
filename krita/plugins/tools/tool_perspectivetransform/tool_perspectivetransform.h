/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
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

#ifndef TOOL_PERSPECTIVE_TRANSFORM_H_
#define TOOL_PERSPECTIVE_TRANSFORM_H_

#include <kparts/plugin.h>

/**
 * A module that provides a tool for doinge perspective transformation.
 */
class ToolPerspectiveTransform : public QObject
{
    Q_OBJECT
public:
    ToolPerspectiveTransform(QObject *parent, const QStringList &);
    virtual ~ToolPerspectiveTransform();

};

#endif // TOOL_PERSPECTIVE_TRANSFORM_H_
