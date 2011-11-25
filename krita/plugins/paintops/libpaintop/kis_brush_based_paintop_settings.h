/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_BRUSH_BASED_PAINTOP_SETTINGS_H
#define KIS_BRUSH_BASED_PAINTOP_SETTINGS_H

#include <kis_paintop_settings.h>
#include <krita_export.h>

class PAINTOP_EXPORT KisBrushBasedPaintOpSettings : public KisPaintOpSettings
{
public:
    ///Reimplemented
    virtual bool paintIncremental();

    ///Reimplemented
    virtual bool isAirbrushing() const;

    ///Reimplemented
    virtual int rate() const;

    virtual QPainterPath brushOutline(const QPointF& pos, OutlineMode mode, qreal scale = 1.0, qreal rotation = 0.0) const;

    ///Reimplemented
    virtual bool isValid();

    ///Reimplemented
    virtual bool isLoadable();
};

#endif // KIS_BRUSH_BASED_PAINTOP_SETTINGS_H
