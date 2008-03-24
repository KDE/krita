/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef KIS_PAINTOP_PRESET_H
#define KIS_PAINTOP_PRESET_H

#include "KoResource.h"

#include "krita_export.h"

class QImage;

/**
 * A KisPaintOpPreset records a particular set of settings
 * associated with a paintop. It differs from KisPaintOpSettings
 * in that though the latter contains much the same information, it
 * also contains state information that should be kept between strokes.
 */
class KRITAIMAGE_EXPORT KisPaintOpPreset : public KoResource
{
public:

    KisPaintOpPreset();
    
    KisPaintOpPreset(const QString& filename);

    ~KisPaintOpPreset();

    bool load();

    bool save();

    QImage img() const;

    bool removable() const { return true; }

    QString defaultFileExtension() const { return "kpp"; }

private:

    /// Called whenever the settings change so the image representing this resource
    /// in the resource choosers is updated.
    void updateImg();

private:
    
    struct Private;
    Private * const m_d;
};

#endif
