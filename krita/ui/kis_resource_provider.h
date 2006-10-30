/*
 *  Copyright (c) 2006 Boudewijn Rempt  <boud@valdyas.org>
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

#ifndef KIS_RESOURCE_PROVIDER_H_
#define KIS_RESOURCE_PROVIDER_H_

#include <QObject>

#include <KoColor.h>
#include <KoID.h>

#include "kis_view2.h"
#include "kis_image.h"

class KisPaintOpSettings;
class KisBrush;
class KisGradient;
class KisPattern;
class KisResource;

/**
   KisResourceProvider contains the per-view current settings that
   influence painting, like paintop, color, gradients and so on.
 */
class KisResourceProvider : public QObject {

    Q_OBJECT

public:

    KisResourceProvider(KisView2 * view)
        : m_view( view )
        , m_brush( 0 )
        , m_pattern( 0 )
        , m_gradient( 0 )
        , m_paintopSettings( 0 )
        , m_HDRExposure( 0 )
        {
            m_fgColor = KoColor(Qt::black, view->image()->colorSpace());
            m_bgColor = KoColor(Qt::white, view->image()->colorSpace());
        }

    KoCanvasBase * canvas() const;

    KoColor bgColor() const;
    void setBGColor(const KoColor& c);

    KoColor fgColor() const;
    void setFGColor(const KoColor& c);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    KisBrush *currentBrush() const;
    KisPattern *currentPattern() const;
    KisGradient *currentGradient() const;

    KoID currentPaintop() const;
    const KisPaintOpSettings *currentPaintopSettings() const;

public slots:

    void slotSetFGColor(const KoColor& c);
    void slotSetBGColor(const KoColor& c);
    void brushActivated(KisResource *brush);
    void patternActivated(KisResource *pattern);
    void gradientActivated(KisResource *gradient);
    void paintopActivated(const KoID & paintop, const KisPaintOpSettings *paintopSettings);

signals:

    void sigFGColorChanged(const KoColor &);
    void sigBGColorChanged(const KoColor &);
    void brushChanged(KisBrush * brush);
    void gradientChanged(KisGradient * gradient);
    void patternChanged(KisPattern * pattern);
    void paintopChanged(KoID paintop, const KisPaintOpSettings *paintopSettings);

private:

    KisView2 * m_view;
    KisBrush * m_brush;
    KisPattern * m_pattern;
    KisGradient * m_gradient;
    KoColor m_fgColor;
    KoColor m_bgColor;
    KoID m_paintop;
    const KisPaintOpSettings *m_paintopSettings;
    float m_HDRExposure;
};

#endif
