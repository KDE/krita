/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPOSTSCRIPTPAINTDEVICE_H
#define KOPOSTSCRIPTPAINTDEVICE_H

#include <QPaintDevice>

#include "kritaflake_export.h"

/**
 * Helper class to disable any screen conversion as that is done in flake.
 *
 * Flake has the property that all content is always defined in pts. And only when it
 * is displayed is it converted to the proper zoom and resolution.
 *
 * This is in contradiction to the normal behavior of Qt fonts which automatically
 * applies DPI on creation.  So this is where this special paint device comes in.
 *
 * Usage;
 *
 * For all QFont() and QFontMetrics constructors add an instance of this PaintDevice
 * to the constructor if those fonts are to be used for painting in a KoShape inheriting
 * class.
 *
 * Note: never try to actually paint on this paint device, since that will noisily crash.
 */
class KRITAFLAKE_EXPORT KoPostscriptPaintDevice : public QPaintDevice
{
public:
    /// constructor
    KoPostscriptPaintDevice();
    /// reimplemented from QPaintDevice
    QPaintEngine *paintEngine() const override;
    /// reimplemented from QPaintDevice
    int metric(QPaintDevice::PaintDeviceMetric metric) const override;
};

#endif
