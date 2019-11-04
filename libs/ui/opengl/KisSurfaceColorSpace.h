/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KISSURFACECOLORSPACE_H
#define KISSURFACECOLORSPACE_H

#include <QSurfaceFormat>

/**
 * This file is a simple workaround for building our surface format
 * selection code on Qt older than 5.10, which doesn't have this feature
 *
 * TODO: remove this file when we drop support for Qt 5.9 LTS
 */

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)

using KisSurfaceColorSpace = QSurfaceFormat::ColorSpace;

#else

enum KisSurfaceColorSpace {
    DefaultColorSpace,
    sRGBColorSpace
};

#endif

#endif // KISSURFACECOLORSPACE_H
