/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CANVAS_RESOURCE_PROVIDER_H_
#define KIS_CANVAS_RESOURCE_PROVIDER_H_

#include <QObject>

#include <KoColor.h>
#include <KoID.h>
#include <KoCanvasResourceProvider.h>
#include <KoResource.h>

#include "kis_types.h"
#include "kritaui_export.h"

#include <KoPattern.h>
#include <KoAbstractGradient.h>
#include <resources/KoGamutMask.h>
#include <kis_workspace_resource.h>
#include "KisPresetShadowUpdater.h"

class KoColorProfile;
class KoAbstractGradient;
struct KoSvgTextPropertyData;

class KoCanvasBase;
class KisViewManager;

class KisFilterConfiguration;

#include <kis_abstract_perspective_grid.h>

/**
 * KisCanvasResourceProvider contains the per-window current settings that
 * influence painting, like paintop, color, gradients and so on.
 */
class KRITAUI_EXPORT KisCanvasResourceProvider : public QObject
{
    Q_OBJECT

public:
    KisCanvasResourceProvider(KisViewManager * view);
    ~KisCanvasResourceProvider() override;

    void setResourceManager(KoCanvasResourceProvider *resourceManager);
    KoCanvasResourceProvider* resourceManager();

    KoCanvasBase * canvas() const;

    KoColor bgColor() const;
    void setBGColor(const KoColor& c);

    KoColor fgColor() const;
    void setFGColor(const KoColor& c);

    QList<KoColor> colorHistory() const;
    void setColorHistory(const QList<KoColor>& colors);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    float HDRGamma() const;
    void setHDRGamma(float gamma);

    bool eraserMode() const;
    void setEraserMode(bool value);

    KoPatternSP currentPattern() const;

    KoAbstractGradientSP currentGradient() const;

    KisImageWSP currentImage() const;

    KisNodeSP currentNode() const;

    KoGamutMaskSP currentGamutMask() const;
    bool gamutMaskActive() const;

    KisPaintOpPresetSP currentPreset() const;
    void setPaintOpPreset(const KisPaintOpPresetSP preset);

    KisPaintOpPresetSP previousPreset() const;
    void setPreviousPaintOpPreset(const KisPaintOpPresetSP preset);

    void setCurrentCompositeOp(const QString& compositeOp);
    QString currentCompositeOp() const;

    void setMirrorHorizontal(bool mirrorHorizontal);
    bool mirrorHorizontal() const;

    void setMirrorVertical(bool mirrorVertical);
    bool mirrorVertical() const;

    // options for horizontal and vertical mirror toolbar
    void setMirrorHorizontalLock(bool isLocked);
    bool mirrorHorizontalLock();
    void setMirrorVerticalLock(bool isLocked);
    bool mirrorVerticalLock();

    void setMirrorVerticalHideDecorations(bool hide);
    bool mirrorVerticalHideDecorations();

    void setMirrorHorizontalHideDecorations(bool hide);
    bool mirrorHorizontalHideDecorations();

    void mirrorVerticalMoveCanvasToCenter();
    void mirrorHorizontalMoveCanvasToCenter();

    void setOpacity(qreal opacity);
    qreal opacity() const;

    void setFlow(qreal opacity);
    qreal flow() const;

    void setSize(qreal size);
    qreal size() const;

    void setFade(qreal fade);
    qreal fade() const;

    void setBrushRotation(qreal rotation);
    qreal brushRotation() const;

    void setPatternSize(qreal size);
    qreal patternSize() const;

    void setGlobalAlphaLock(bool lock);
    bool globalAlphaLock() const;

    void setDisablePressure(bool value);
    bool disablePressure() const;

    void setTextPropertyData(KoSvgTextPropertyData data);
    KoSvgTextPropertyData textPropertyData() const;

    void setCharacterPropertyData(KoSvgTextPropertyData data);
    KoSvgTextPropertyData characterTextPropertyData() const;

    ///Notify that the workspace is saved and settings should be saved to it
    void notifySavingWorkspace(KisWorkspaceResourceSP workspace);

    ///Notify that the workspace is loaded and settings can be read
    void notifyLoadingWorkspace(KisWorkspaceResourceSP workspace);

public Q_SLOTS:

    void slotSetFGColor(const KoColor& c);
    void slotSetBGColor(const KoColor& c);
    void slotPatternActivated(KoResourceSP pattern);
    void slotGradientActivated(KoResourceSP gradient);
    void slotNodeActivated(const KisNodeSP node);
    void slotPainting();

    void slotGamutMaskActivated(KoGamutMaskSP mask);
    void slotGamutMaskUnset();
    void slotGamutMaskPreviewUpdate();
    void slotGamutMaskDeactivate();

    /**
     * Set the image size in pixels. The resource provider will store
     * the image size in postscript points.
     */
    // FIXME: this slot doesn't catch the case when image resolution is changed
    void slotImageSizeChanged();
    void slotOnScreenResolutionChanged();

private Q_SLOTS:

    void slotCanvasResourceChanged(int key, const QVariant & res);

Q_SIGNALS:

    void sigPaintOpPresetChanged(const KisPaintOpPresetSP);
    void sigFGColorChanged(const KoColor &);
    void sigBGColorChanged(const KoColor &);
    void sigGradientChanged(KoAbstractGradientSP);
    void sigPatternChanged(KoPatternSP);
    void sigNodeChanged(const KisNodeSP);
    void sigFGColorUsed(const KoColor&);
    void sigOnScreenResolutionChanged(qreal scaleX, qreal scaleY);
    void sigEffectiveCompositeOpChanged();
    void sigOpacityChanged(qreal);
    void sigSavingWorkspace(KisWorkspaceResourceSP workspace);
    void sigLoadingWorkspace(KisWorkspaceResourceSP workspace);

    void mirrorModeChanged();
    void moveMirrorVerticalCenter();
    void moveMirrorHorizontalCenter();

    void sigGamutMaskChanged(KoGamutMaskSP mask);
    void sigGamutMaskUnset();
    void sigGamutMaskPreviewUpdate();
    void sigGamutMaskDeactivated();

    void sigTextPropertiesChanged();
    void sigCharacterPropertiesChanged();

private:

    KisViewManager * m_view {nullptr};
    KoCanvasResourceProvider *m_resourceManager {nullptr};
    bool m_fGChanged {true};
    KisPresetShadowUpdater m_presetShadowUpdater;
};

#endif
