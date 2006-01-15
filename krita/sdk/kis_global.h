/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KISGLOBAL_H_
#define KISGLOBAL_H_

#include <config.h>
#include LCMS_HEADER
#include <limits.h>
#include <qglobal.h>
#include <kglobal.h>

#define KRITA_VERSION VERSION

const Q_UINT8 Q_UINT8_MAX = UCHAR_MAX;
const Q_UINT16 Q_UINT16_MAX = 65535;

const Q_INT32 Q_INT32_MAX = (2147483647);
const Q_INT32 Q_INT32_MIN = (-2147483647-1);

const Q_UINT8 OPACITY_TRANSPARENT = 0;
const Q_UINT8 OPACITY_OPAQUE = UCHAR_MAX;

const Q_UINT8 MAX_SELECTED = UCHAR_MAX;
const Q_UINT8 MIN_SELECTED = 0;
const Q_UINT8 SELECTION_THRESHOLD = 1;

enum enumCursorStyle {
    CURSOR_STYLE_TOOLICON = 0,
    CURSOR_STYLE_CROSSHAIR = 1,
    CURSOR_STYLE_POINTER = 2,
    CURSOR_STYLE_OUTLINE = 3
};

enum enumResourceTypes {
    RESOURCE_PAINTOP,
    RESOURCE_FILTER,
    RESOURCE_TOOL,
    RESOURCE_COLORSPACE
};

/*
 * Most wacom pads have 512 levels of pressure; Qt only supports 256, and even
 * this is downscaled to 127 levels because the line would be too jittery, and
 * the amount of masks take too much memory otherwise.
 */
const Q_INT32 PRESSURE_LEVELS= 127;
const double PRESSURE_MIN = 0.0;
const double PRESSURE_MAX = 1.0;
const double PRESSURE_DEFAULT = (PRESSURE_MAX - PRESSURE_MIN) / 2;
const double PRESSURE_THRESHOLD = 5.0 / 255.0;

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))


namespace krita {

    // String constants for palettes and palette widgets
    const QString TOOL_OPTION_WIDGET ("tooloptions");

    const QString CONTROL_PALETTE ("controlpalette");
    const QString PAINTBOX ("paintbox");
    const QString COLORBOX ("colorbox");
    const QString LAYERBOX ("layerbox");
}

#endif // KISGLOBAL_H_

