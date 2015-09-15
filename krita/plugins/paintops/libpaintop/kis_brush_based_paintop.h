/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_BRUSH_BASED_PAINTOP_H
#define KIS_BRUSH_BASED_PAINTOP_H

#include "kritapaintop_export.h"
#include "kis_paintop.h"
#include "kis_dab_cache.h"
#include "kis_brush.h"
#include "kis_texture_option.h"
#include "kis_precision_option.h"
#include "kis_pressure_mirror_option.h"
#include <kis_threaded_text_rendering_workaround.h>


class KisPropertiesConfiguration;
class KisPressureSpacingOption;

/// Internal
class TextBrushInitializationWorkaround
{
public:
    TextBrushInitializationWorkaround();
    ~TextBrushInitializationWorkaround();
    static TextBrushInitializationWorkaround* instance();

    void preinitialize(const KisPropertiesConfiguration *settings);

    KisBrushSP tryGetBrush(const KisPropertiesConfiguration *settings);


private:
    KisBrushSP m_brush;
    const KisPropertiesConfiguration *m_settings;
};


/**
 * This is a base class for paintops that use a KisBrush or derived
 * brush to paint with. This is mainly important for the spacing
 * generation.
 */
class PAINTOP_EXPORT KisBrushBasedPaintOp : public KisPaintOp
{

public:

    KisBrushBasedPaintOp(const KisPropertiesConfiguration* settings, KisPainter* painter);
    ~KisBrushBasedPaintOp();

    bool checkSizeTooSmall(qreal scale);

    KisSpacingInformation effectiveSpacing(qreal scale, qreal rotation) const;
    KisSpacingInformation effectiveSpacing(qreal scale, qreal rotation, const KisPressureSpacingOption &spacingOption, const KisPaintInformation &pi) const;

    ///Reimplemented, false if brush is 0
    virtual bool canPaint() const;

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    typedef int needs_preinitialization;
    static void preinitializeOpStatically(const KisPaintOpSettingsSP settings);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

private:
    KisSpacingInformation effectiveSpacing(qreal dabWidth, qreal dabHeight, qreal extraScale, bool isotropicSpacing, qreal rotation) const;

protected: // XXX: make private!

    KisBrushSP m_brush;
    KisTextureProperties m_textureProperties;
    KisPressureMirrorOption m_mirrorOption;
    KisPrecisionOption m_precisionOption;
    KisDabCache *m_dabCache;
};

#endif
