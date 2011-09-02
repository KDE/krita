/* Part of Calligra Suite - Marble Map Shape
   Copyright 2008 Simon Schmeisser <mail_to_wrt@gmx.de>
   Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef GEO_SHAPE_SET_MAP_THEME_ID_COMMAND
#define GEO_SHAPE_SET_MAP_THEME_ID_COMMAND

#include <global.h>

#include <kundo2command.h>

class MarbleMapShape;

/// The undo / redo command for setting the map theme
class MarbleMapShapeCommandSetMapThemeId : public KUndo2Command
{
public:
    /**
    * Command to set the MapTheme that is used.
    *
    * @param shape the shape where we want to zoom
    * @param mapThemeId MapTheme that is wanted
    * @param parent the parent command used for macro commands
     */
    MarbleMapShapeCommandSetMapThemeId(MarbleMapShape * shape, const QString& MapThemeId, KUndo2Command *parent = 0);

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

    private:
    MarbleMapShape *m_shape;
    QString m_old_mapThemeId;
    QString m_new_mapThemeId;
};


#endif
