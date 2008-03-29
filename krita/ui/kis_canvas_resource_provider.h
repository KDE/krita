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

#include <kis_paintop_settings.h>
#include "kis_types.h"
#include "krita_export.h"

class KisView2;
class KisBrush;
class KoAbstractGradient;
class KisPattern;
class KoResource;
class KoCanvasBase;
class KisPaintOpPreset;

/**
 * KisCanvasResourceProvider contains the per-view current settings that
 * influence painting, like paintop, color, gradients and so on.
 */
class KRITAUI_EXPORT KisCanvasResourceProvider : public QObject {

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
        CurrentDisplayProfile,
        CurrentImage,
        CurrentKritaNode,
        CurrentPaintOpPreset
    };


    KisCanvasResourceProvider(KisView2 * view);
    ~KisCanvasResourceProvider();

    void setCanvasResourceProvider( KoCanvasResourceProvider * resourceProvider );

    KoCanvasBase * canvas() const;

    KoColor bgColor() const;
    void setBGColor(const KoColor& c);

    KoColor fgColor() const;
    void setFGColor(const KoColor& c);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    KisBrush *currentBrush() const;
    KisPattern *currentPattern() const;
    KoAbstractGradient *currentGradient() const;

    KoID currentPaintop() const;
    const KisPaintOpSettingsSP currentPaintopSettings() const;

    KisLayerSP currentLayer() const;

    void resetDisplayProfile();
    const KoColorProfile * currentDisplayProfile() const;

    KisImageSP currentImage() const;

    KisNodeSP currentNode() const;

    //KisPaintOpPreset * currentPreset() const;

public slots:

    void slotSetFGColor(const KoColor& c);
    void slotSetBGColor(const KoColor& c);
    void slotBrushActivated(KoResource *brush);
    void slotPatternActivated(KoResource *pattern);
    void slotGradientActivated(KoResource *gradient);
    void slotPaintopActivated(const KoID & paintop, const KisPaintOpSettingsSP paintopSettings);
    void slotLayerActivated( const KisLayerSP layer );
    void slotNodeActivated( const KisNodeSP node );

    /**
     * Set the image size in pixels. The resource provider will store
     * the image size in postscript points.
     */
    void slotSetImageSize( qint32 w, qint32 h );
    void slotSetDisplayProfile( const KoColorProfile * profile );

private slots:

    // TODO:  this method is not called or connected. (TZ)
    void slotResourceChanged( int key, const QVariant & res );

signals:

    void sigFGColorChanged(const KoColor &);
    void sigBGColorChanged(const KoColor &);
    void sigBrushChanged(KisBrush * brush);
    void sigGradientChanged(KoAbstractGradient *);
    void sigPatternChanged(KisPattern *);
    void sigPaintopChanged(KoID paintop, const KisPaintOpSettingsSP);
    void sigLayerChanged(const KisLayerSP);
    void sigNodeChanged(const KisNodeSP);
    void sigDisplayProfileChanged(const KoColorProfile *);

private:

    KisView2 * m_view;
    KoCanvasResourceProvider * m_resourceProvider;
    KisBrush * m_defaultBrush;
    const KoColorProfile * m_displayProfile;

};

#endif
