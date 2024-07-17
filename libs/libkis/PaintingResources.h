/*
 *  Copyright (c) 2020 Scott Petrovic <scottpetrovic@gmail.com>
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
#ifndef LIBKIS_PAINTINGRESOURCES_H
#define LIBKIS_PAINTINGRESOURCES_H

#include <QObject>
#include <QColor>
#include <kis_types.h>

#include "kritalibkis_export.h"
#include "KoCanvasResourceProvider.h"
#include "kis_stroke_strategy.h"

#include <kis_figure_painting_tool_helper.h>

#include "libkis.h"

#include "View.h"

/**
 * @brief The PaintingResources namespace
 * Sets up information related to making painting strokes.
 * Used primarily in the Document class
 *
 */
namespace PaintingResources
{
    KisFigurePaintingToolHelper createHelper(KisImageWSP image);
};

#endif // LIBKIS_PAINTINGRESOURCES_H
