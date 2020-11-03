/*
 *  Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
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


#ifndef KIS_FILE_ICON_CREATOR_H
#define KIS_FILE_ICON_CREATOR_H

#include <QScopedPointer>
#include <QIcon>

#include "kritaui_export.h"

/**
 * @brief The KisFileIconCreator class creates a thumbnail from a file on disk
 *
 * On Welcome Page and possibly other places there might be a need to show the user
 * a thumbnail of a file. This class tries to open a file and create a thumbnail out of it.
 *
 * In theory creating the object is not needed, so if you, dear future reader, want to convert
 * the function inside to a static one, go ahead.
 *
 */
class KRITAUI_EXPORT KisFileIconCreator
{

public:
    /**
     * @brief KisFileIconCreator basic constructor
     */
    KisFileIconCreator();

    /**
     * @brief createFileIcon creates an icon from the file on disk
     * @param path path to the file
     * @param icon created icon
     * @param devicePixelRatioF a result from devicePixelRatioF() called in a widget
     * @param iconSize size of the icon
     * @param crop make the square icon - zoom crop icon to the minimum side
     * @return true if icon was created successfully, false if not (for example the file doesn't exist)
     */
    bool createFileIcon(QString path, QIcon &icon, qreal devicePixelRatioF, QSize iconSize, bool crop);
};

#endif // KIS_FILE_ICON_CREATOR_H
