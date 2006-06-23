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
#include <config-krita.h>
#include <lcms.h>
#include <limits.h>

#include <QString>

#include <kglobal.h>

#define KRITA_VERSION VERSION

const quint8 quint8_MAX = UCHAR_MAX;
const quint16 quint16_MAX = 65535;

const qint32 qint32_MAX = (2147483647);
const qint32 qint32_MIN = (-2147483647-1);

const quint8 MAX_SELECTED = UCHAR_MAX;
const quint8 MIN_SELECTED = 0;
const quint8 SELECTION_THRESHOLD = 1;

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
const qint32 PRESSURE_LEVELS= 127;
const double PRESSURE_MIN = 0.0;
const double PRESSURE_MAX = 1.0;
const double PRESSURE_DEFAULT = (PRESSURE_MAX - PRESSURE_MIN) / 2;
const double PRESSURE_THRESHOLD = 5.0 / 255.0;


namespace krita {

    // String constants for palettes and palette widgets
    const QString TOOL_OPTION_WIDGET ("tooloptions");

    const QString CONTROL_PALETTE ("controlpalette");
    const QString PAINTBOX ("paintbox");
    const QString COLORBOX ("colorbox");
    const QString LAYERBOX ("layerbox");
    const QString NEWLAYERBOX ("newlayerbox");
}

#endif // KISGLOBAL_H_

