/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef STACKEDPRESET_H
#define STACKEDPRESET_H

#include <QString>
#include <kis_paintop_preset.h>

struct StackedPreset
{
    StackedPreset();

    QString presetName;
    KisPaintOpPresetSP paintopPreset;

    double fuzziness;
    double verticalOffset;
    double horizontalOffset;
    double opacitiy;
    QString compositeOp;
};

Q_DECLARE_METATYPE(StackedPreset)

#endif // STACKEDPRESET_H
