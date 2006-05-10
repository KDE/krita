/* 
 * kis_texture_filter.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef _TEXTURE_FILTER_H
#define _TEXTURE_FILTER_H

#include <QString>
#include <klocale.h>
#include <kis_paint_device_action.h>

/// Initializes a wet paint device with a texture
class WetPaintDevAction : public KisPaintDeviceAction {
public:
    virtual ~WetPaintDevAction() {}

    virtual void act(KisPaintDeviceSP device, qint32 w = 0, qint32 h = 0) const;
    virtual QString name() const { return i18n("Wet Texture"); }
    virtual QString description() const { return i18n("Add a texture to the wet canvas"); }
};

#endif // _TEXTURE_FILTER_H
