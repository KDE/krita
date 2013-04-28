/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_DEFORM_PAINTOP_SETTINGS_H_
#define KIS_DEFORM_PAINTOP_SETTINGS_H_

class KisDeformPaintOpSettingsWidget;

#include <kis_paintop_settings.h>
#include <kis_types.h>

#include <opengl/kis_opengl.h>

#if defined(_WIN32) || defined(_WIN64)
#ifndef __MINGW32__
# include <windows.h>
#endif
#endif

class KisDeformPaintOpSettings : public KisPaintOpSettings
{

public:
    KisDeformPaintOpSettings(){}
    virtual ~KisDeformPaintOpSettings() {}

    virtual QPainterPath brushOutline(const QPointF& pos, OutlineMode mode, qreal scale = 1.0, qreal rotation = 0.0) const;
    
    bool paintIncremental();
    bool isAirbrushing() const;
    int rate() const;

#if defined(HAVE_OPENGL)
    inline QString modelName() const {
        return "3d-deform-brush";
    }
#endif

private:
    KisDeformPaintOpSettingsWidget* m_options;
};
#endif
