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
#include "krita_export.h"
#include "kis_complex_color.h"

class KisPaintOpSettings;
class KisBrush;
class KisGradient;
class KisPattern;
class KoResource;

/**
   KisResourceProvider contains the per-view current settings that
   influence painting, like paintop, color, gradients and so on.

   XXX: KisBrush, KisGradient, KisPattern and the other pointers
   should really be shared pointers. That would be much safer. Also
   note: we should have a koffice-wide provider of brushes, patterns
   and gradients.

   XXX: CurrentKritaLayer should be accompanied by
   CurrentKritaPaintDevice and CurrentKritaMask, because masks are also important for tools

 */
class KRITAUI_EXPORT KisResourceProvider : public QObject {

    Q_OBJECT

public:

    enum Resources {
        HdrExposure = KoCanvasResource::KritaStart+1,
        CurrentBrush,
        CurrentPattern,
        CurrentGradient,
        CurrentPaintop,
        CurrentPaintopSettings,
        CurrentKritaLayer,
        CurrentComplexColor,
        CurrentDisplayProfile,
    };


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

    KisComplexColor *currentComplexColor() const;

    KoColorProfile * currentDisplayProfile() const;

public slots:

    void slotSetFGColor(const KoColor& c);
    void slotSetBGColor(const KoColor& c);
    void slotBrushActivated(KoResource *brush);
    void slotPatternActivated(KoResource *pattern);
    void slotGradientActivated(KoResource *gradient);
    void slotPaintopActivated(const KoID & paintop, const KisPaintOpSettings *paintopSettings);
    void slotLayerActivated( const KisLayerSP layer );

    /**
     * Set the image size in pixels. The resource provider will store
     * the image size in postscript points.
     */
    void slotSetImageSize( qint32 w, qint32 h );
    void slotSetDisplayProfile( KoColorProfile * profile );

private slots:

    // TODO:  this method is not called or connected. (TZ)
    void slotResourceChanged( int key, const QVariant & res );

signals:

    void sigFGColorChanged(const KoColor &);
    void sigBGColorChanged(const KoColor &);
    void sigBrushChanged(KisBrush * brush);
    void sigGradientChanged(KisGradient * gradient);
    void sigPatternChanged(KisPattern * pattern);
    void sigPaintopChanged(KoID paintop, const KisPaintOpSettings *paintopSettings);
    void sigLayerChanged( const KisLayerSP layer );
    void sigDisplayProfileChanged( const KoColorProfile * profile );

private:

    KisView2 * m_view;
    KoCanvasResourceProvider * m_resourceProvider;
    KisBrush * m_defaultBrush;
    KisComplexColor *m_defaultComplex;
    KoColorProfile * m_displayProfile;

};

#endif
