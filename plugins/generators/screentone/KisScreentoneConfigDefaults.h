/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISSCREENTONECONFIGDEFAULTS_H
#define KISSCREENTONECONFIGDEFAULTS_H

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include <QtGlobal>

namespace KisScreentoneConfigDefaults
{

constexpr int pattern() { return 0; }
constexpr int shape() { return 0; }
constexpr int interpolation() { return 0; }

inline const KoColor& foregroundColor()
{
    static const KoColor c(Qt::black, KoColorSpaceRegistry::instance()->rgb8());
    return c;
}

inline const KoColor& backgroundColor()
{
    static const KoColor c(Qt::white, KoColorSpaceRegistry::instance()->rgb8());
    return c;
}

constexpr int foregroundOpacity() { return 100; }
constexpr int backgroundOpacity() { return 100; }
constexpr bool invert() { return false; }
constexpr qreal brightness() { return 50.0; }
constexpr qreal contrast() { return 50.0; }

constexpr qreal positionX() { return 0.0; }
constexpr qreal positionY() { return 0.0; }
constexpr qreal sizeX() { return 10.0; }
constexpr qreal sizeY() { return 10.0; }
constexpr bool keepSizeSquare() { return true; }
constexpr qreal shearX() { return 0.0; }
constexpr qreal shearY() { return 0.0; }
constexpr qreal rotation() { return 0.0; }

}

#endif
