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

#ifndef KIS_CANVAS_RESOURCE_PROVIDER_H_
#define KIS_CANVAS_RESOURCE_PROVIDER_H_

#include <QObject>

#include <KoColor.h>
#include <KoID.h>
#include <KoCanvasResourceManager.h>

#include "kis_types.h"
#include "krita_export.h"

class KisWorkspaceResource;
class KoColorProfile;
class KoAbstractGradient;
class KoResource;

class KoCanvasBase;
class KisView2;
class KisPattern;
class KisFilterConfiguration;
class KisAbstractPerspectiveGrid;

/**
 * KisCanvasResourceProvider contains the per-view current settings that
 * influence painting, like paintop, color, gradients and so on.
 */
class KRITAUI_EXPORT KisCanvasResourceProvider : public QObject
{

    Q_OBJECT

public:

    enum Resources {
        HdrExposure = KoCanvasResourceManager::KritaStart + 1,
        CurrentPattern,
        CurrentGradient,
        CurrentDisplayProfile,
        CurrentImage,
        CurrentKritaNode,
        CurrentPaintOpPreset,
        CurrentGeneratorConfiguration,
        CurrentCompositeOp,
        MirrorHorizontal,
        MirrorVertical,
        MirrorAxisCenter,
        Opacity,
        HdrGamma
    };


    KisCanvasResourceProvider(KisView2 * view);
    ~KisCanvasResourceProvider();

    void setResourceManager(KoCanvasResourceManager *resourceManager);
    KoCanvasResourceManager* resourceManager();

    KoCanvasBase * canvas() const;

    KoColor bgColor() const;
    void setBGColor(const KoColor& c);

    KoColor fgColor() const;
    void setFGColor(const KoColor& c);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    float HDRGamma() const;
    void setHDRGamma(float gamma);

    KisPattern *currentPattern() const;

    KoAbstractGradient *currentGradient() const;

    void resetDisplayProfile(int screen = -1);
    const KoColorProfile * currentDisplayProfile() const;

    KisImageWSP currentImage() const;

    KisNodeSP currentNode() const;

    KisPaintOpPresetSP currentPreset() const;

    KisFilterConfiguration* currentGeneratorConfiguration() const;

    void setCurrentCompositeOp(const QString& compositeOp);
    QString currentCompositeOp() const;

    QList<KisAbstractPerspectiveGrid*> perspectiveGrids() const;
    void addPerspectiveGrid(KisAbstractPerspectiveGrid*);
    void removePerspectiveGrid(KisAbstractPerspectiveGrid*);
    void clearPerspectiveGrids();

    void setMirrorHorizontal(bool mirrorHorizontal);
    bool mirrorHorizontal() const;

    void setMirrorVertical(bool mirrorVertical);
    bool mirrorVertical() const;

    void setOpacity(qreal opacity);
    qreal opacity();

    void setPaintOpPreset(const KisPaintOpPresetSP preset);

    ///Notify that the workspace is saved and settings should be saved to it
    void notifySavingWorkspace(KisWorkspaceResource* workspace);

    ///Notify that the workspace is loaded and settings can be read
    void notifyLoadingWorkspace(KisWorkspaceResource* workspace);

public slots:

    void slotSetFGColor(const KoColor& c);
    void slotSetBGColor(const KoColor& c);
    void slotPatternActivated(KoResource *pattern);
    void slotGradientActivated(KoResource *gradient);
    void slotNodeActivated(const KisNodeSP node);
    void slotGeneratorConfigurationActivated(KisFilterConfiguration * generatorConfiguration);
    void slotPainting();

    /**
     * Set the image size in pixels. The resource provider will store
     * the image size in postscript points.
     */
    // FIXME: this slot doesn't catch the case when image resolution is changed
    void slotImageSizeChanged();
    void slotSetDisplayProfile(const KoColorProfile * profile);

    void slotOnScreenResolutionChanged();

    // This is a flag to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent color if the pop up palette
    // is not visible
    void slotResetEnableFGChange(bool);

private slots:

    void slotResourceChanged(int key, const QVariant & res);

signals:

    void sigFGColorChanged(const KoColor &);
    void sigBGColorChanged(const KoColor &);
    void sigGradientChanged(KoAbstractGradient *);
    void sigPatternChanged(KisPattern *);
    void sigPaintOpPresetChanged(KisPaintOpPresetSP preset);
    void sigNodeChanged(const KisNodeSP);
    void sigDisplayProfileChanged(const KoColorProfile *);
    void sigGeneratorConfigurationChanged(KisFilterConfiguration * generatorConfiguration);
    void sigFGColorUsed(const KoColor&);
    void sigCompositeOpChanged(const QString &);
    void sigOnScreenResolutionChanged(qreal scaleX, qreal scaleY);
    void sigOpacityChanged(qreal);
    void sigSavingWorkspace(KisWorkspaceResource* workspace);
    void sigLoadingWorkspace(KisWorkspaceResource* workspace);

private:

    KisView2 * m_view;
    KoCanvasResourceManager *m_resourceManager;
    const KoColorProfile *m_displayProfile;
    bool m_fGChanged;
    QList<KisAbstractPerspectiveGrid*> m_perspectiveGrids;

    // This is a flag to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent color if the pop up palette
    // is not visible
    bool m_enablefGChange;

};

#endif
