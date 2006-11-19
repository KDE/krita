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
#include <KoCanvasResourceProvider.h>

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

   XXX: KisBrush, KisGradient, KisPattern and the other pointers
   should really be shared pointers. That would be much safer. Also
   note: we should have a koffice-wide provider of brushes, patterns
   and gradients.

 */
class KisResourceProvider : public QObject {

    Q_OBJECT

public:

    KisResourceProvider(KisView2 * view);
    ~KisResourceProvider();

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

    KisLayerSP currentLayer() const;


public slots:

    void slotSetFGColor(const KoColor& c);
    void slotSetBGColor(const KoColor& c);
    void slotBrushActivated(KisResource *brush);
    void slotPatternActivated(KisResource *pattern);
    void slotGradientActivated(KisResource *gradient);
    void slotPaintopActivated(const KoID & paintop, const KisPaintOpSettings *paintopSettings);
    void slotLayerActivated( const KisLayerSP layer );

private slots:

    void slotResourceChanged( KoCanvasResource::EnumCanvasResource key, const QVariant & res );

signals:

    void sigFGColorChanged(const KoColor &);
    void sigBGColorChanged(const KoColor &);
    void sigBrushChanged(KisBrush * brush);
    void sigGradientChanged(KisGradient * gradient);
    void sigPatternChanged(KisPattern * pattern);
    void sigPaintopChanged(KoID paintop, const KisPaintOpSettings *paintopSettings);
    void sigLayerChanged( const KisLayerSP layer );

private:

    KisView2 * m_view;
    KoCanvasResourceProvider * m_resourceProvider;
    KisBrush * m_defaultBrush;
};

#endif
