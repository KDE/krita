/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_TOOL_H_
#define KIS_TOOL_H_

/// Defintions of the toolgroups of Krita

static const QString TOOL_TYPE_SHAPE = "Krita/Shape"; // Geometric shapes like ellipses and lines
static const QString TOOL_TYPE_FREEHAND = "Krita/Freehand"; // Freehand drawing tools
static const QString TOOL_TYPE_TRANSFORM = "Krita/Transform"; // Tools that transform the layer;
static const QString TOOL_TYPE_FILL = "Krita/Fill"; // Tools that fill parts of the canvas
static const QString TOOL_TYPE_VIEW = "Krita/View"; // Tools that affect the canvas: pan, zoom, etc.
static const QString TOOL_TYPE_SELECTED = "Krita/Select"; // Tools that select pixels

#endif // KIS_TOOL_H_

