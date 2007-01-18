/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOPOSTSCRIPTPAINTDEVICE_H
#define KOPOSTSCRIPTPAINTDEVICE_H

#include <QPaintDevice>

#include <flake_export.h>

/**
 * helper class to disable any screen convertion as thats done in flake.
 * Flake has the property that all content is always defined in pts. And only when it
 * is displayed is it converted to the proper zoom and resolution.
 * This is in contradiction to the normal behavior of Qt fonts which automatically
 * applies DPI on creation.  So this is where this special paint device comes in.
 * Usage;
 * For all QFont() and QFontMetrics constructors add an instance of this PaintDevice
 * to the constructor if those fonts are to be used for painting in a KoShape inherting
 * class.
 */
class FLAKE_EXPORT KoPostscriptPaintDevice : public QPaintDevice {
public:
    /// constructor
    KoPostscriptPaintDevice();
    /// reimplemented from QPaintDevice
    QPaintEngine *paintEngine () const;
    /// reimplemented from QPaintDevice
    int metric (QPaintDevice::PaintDeviceMetric metric) const;
};

#endif
