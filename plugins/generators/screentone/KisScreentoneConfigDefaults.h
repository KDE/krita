/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
constexpr qreal contrast() { return 95.0; }

constexpr qreal positionX() { return 0.0; }
constexpr qreal positionY() { return 0.0; }
constexpr qreal sizeX() { return 10.0; }
constexpr qreal sizeY() { return 10.0; }
constexpr bool keepSizeSquare() { return true; }
constexpr qreal shearX() { return 0.0; }
constexpr qreal shearY() { return 0.0; }
constexpr qreal rotation() { return 45.0; }

}

#endif
