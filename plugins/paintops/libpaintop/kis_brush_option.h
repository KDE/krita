/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2008
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

#ifndef KIS_BRUSH_OPTION_H_
#define KIS_BRUSH_OPTION_H_

#include <kis_brush.h>
#include <KisPaintopPropertiesBase.h>
#include <kis_properties_configuration.h>
#include <kis_threaded_text_rendering_workaround.h>


#include <kritapaintop_export.h>

class PAINTOP_EXPORT KisBrushOptionProperties : public KisPaintopPropertiesBase
{
public:

    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override;
    void readOptionSettingImpl(const KisPropertiesConfiguration *setting) override;

    KisBrushSP brush() const;
    void setBrush(KisBrushSP brush);

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    static bool isTextBrush(const KisPropertiesConfiguration *setting);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

private:
    KisBrushSP m_brush;
};

#endif
