/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef TOOL_CROP_H_
#define TOOL_CROP_H_

#include <QObject>
#include <QVariant>

/**
 * A module that provides a crop tool.
 */
class ToolCrop : public QObject
{
    Q_OBJECT
public:
    ToolCrop(QObject *parent, const QVariantList &);
    virtual ~ToolCrop();

};

#endif // TOOL_CROP_H_
