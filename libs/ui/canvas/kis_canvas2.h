/* This file is part of the KDE project
 * Copyright (C) 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2011       Silvio Heinrich <plassy@web.de>
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

#ifndef KIS_CANVAS_H
#define KIS_CANVAS_H

#include <QObject>
#include <QWidget>
#include <QSize>
#include <QString>

#include <KoConfig.h>
#include <KoColorConversionTransformation.h>
#include <KoCanvasBase.h>
#include <kritaui_export.h>
#include <kis_types.h>
#include <KoPointerEvent.h>

#include "opengl/kis_opengl.h"

#include "kis_ui_types.h"
#include "kis_coordinates_converter.h"
#include "kis_canvas_decoration.h"
#include "kis_painting_assistants_decoration.h"
#include "input/KisInputActionGroup.h"
#include "KisReferenceImagesDecoration.h"

class KoToolProxy;
class KoColorProfile;


class KisViewManager;
class KisFavoriteResourceManager;
class KisDisplayFilter;
class KisDisplayColorConverter;
struct KisExposureGammaCorrectionInterface;
class KisView;
class KisInputManager;
class KisAnimationPlayer;
class KisShapeController;
class KisCoordinatesConverter;
class KoViewConverter;
class KisAbstractCanvasWidget;

/**
 * KisCanvas2 is not an actual widget class, but rather an adapter for
 * the widget it contains, which may be either a QPainter based
 * canvas, or an OpenGL based canvas: that are the real widgets.
 */
class KRITAUI_EXPORT KisCanvas2 : public KoCanvasBase, public KisInputActionGroupsMaskInterface
{

    Q_OBJECT

public:

    /**
     * Create a new canvas. The canvas manages a widget that will do
     * the actual painting: the canvas itself is not a widget.
     *
     * @param viewConverter the viewconverter for converting between
     *                       window and document coordinates.
     */
    KisCanvas2(KisCoordinatesConverter *coordConverter, KoCanvasResourceProvider *resourceManager, KisView *view, KoShapeControllerBase *sc);

    ~KisCanvas2() override;

    void notifyZoomChanged();

    void disconnectCanvasObserver(QObject *object) override;

public: // KoCanvasBase implementation

    bool canvasIsOpenGL() const override;

    KisOpenGL::FilterMode openGLFilterMode() const;

    void gridSize(QPointF *offset, QSizeF *spacing) const override;

    bool snapToGrid() const override;

    // This method only exists to support flake-related operations
    void addCommand(KUndo2Command *command) override;

    QPoint documentOrigin() const override;
    QPoint documentOffset() const;

    /**
     * Return the right shape manager for the current layer. That is
     * to say, if the current layer is a vector layer, return the shape
     * layer's canvas' shapemanager, else the shapemanager associated
     * with the global krita canvas.
     */
    KoShapeManager * shapeManager() const override;

    /**
     * Since shapeManager() may change, we need a persistent object where we can
     * connect to and thack the selection. See more comments in KoCanvasBase.
     */
    KoSelectedShapesProxy *selectedShapesProxy() const override;

    /**
     * Return the shape manager associated with this canvas
     */
    KoShapeManager *globalShapeManager() const;

    /**
     * Return shape manager associated with the currently active node.
     * If current node has no internal shape manager, return null.
     */
    KoShapeManager *localShapeManager() const;


    void updateCanvas(const QRectF& rc) override;

    void updateInputMethodInfo() override;

    const KisCoordinatesConverter* coordinatesConverter() const;
    KoViewConverter *viewConverter() const override;

    QWidget* canvasWidget() override;

    const QWidget* canvasWidget() const override;

    KoUnit unit() const override;

    KoToolProxy* toolProxy() const override;

    const KoColorProfile* monitorProfile();

    // FIXME:
    // Temporary! Either get the current layer and image from the
    // resource provider, or use this, which gets them from the
    // current shape selection.
    KisImageWSP currentImage() const;

    /**
     * Filters events and sends them to canvas actions. Shared
     * among all the views/canvases
     *
     * NOTE: May be null while initialization!
     */
    KisInputManager* globalInputManager() const;

    /**
     * Return the mask of currently available input action groups
     * Note: Override from KisInputActionGroupsMaskInterface
     */
    KisInputActionGroupsMask inputActionGroupsMask() const override;

    /**
     * Set the mask of currently available action groups
     * Note: Override from KisInputActionGroupsMaskInterface
     */
    void setInputActionGroupsMask(KisInputActionGroupsMask mask) override;

    KisPaintingAssistantsDecorationSP paintingAssistantsDecoration() const;
    KisReferenceImagesDecorationSP referenceImagesDecoration() const;

public: // KisCanvas2 methods

    KisImageWSP image() const;
    KisViewManager* viewManager() const;
    QPointer<KisView> imageView() const;

    /// @return true if the canvas image should be displayed in vertically mirrored mode
    void addDecoration(KisCanvasDecorationSP deco);
    KisCanvasDecorationSP decoration(const QString& id) const;

    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter);
    QSharedPointer<KisDisplayFilter> displayFilter() const;

    KisDisplayColorConverter *displayColorConverter() const;
    KisExposureGammaCorrectionInterface* exposureGammaCorrectionInterface() const;

    /**
     * @brief setProofingOptions
     * set the options for softproofing, without affecting the proofing options as stored inside the image.
     */
    void setProofingOptions(bool softProof, bool gamutCheck);
    KisProofingConfigurationSP proofingConfiguration() const;

    /**
     * @brief setProofingConfigUpdated This function is to set whether the proofing config is updated,
     * this is needed for determining whether or not to generate a new proofing transform.
     * @param updated whether it's updated. Just set it to false in normal usage.
     */
    void setProofingConfigUpdated(bool updated);

    /**
     * @brief proofingConfigUpdated ask the canvas whether or not it updated the proofing config.
     * @return whether or not the proofing config is updated, if so, a new proofing transform needs to be made
     * in KisOpenGL canvas.
     */
    bool proofingConfigUpdated();

    void setCursor(const QCursor &cursor) override;
    KisAnimationFrameCacheSP frameCache() const;
    KisAnimationPlayer *animationPlayer() const;
    void refetchDataFromImage();

    /**
     * @return area of the image (in image coordinates) that is visible on the canvas
     * with a small margin selected by the user
     */
    QRect regionOfInterest() const;

    /**
     * Set artificial limit outside which the image will not be rendered
     * \p rc is measured in image pixels
     */
    void setRenderingLimit(const QRect &rc);

    /**
     * @return aftificial limit outside which the image will not be rendered
     */
    QRect renderingLimit() const;

Q_SIGNALS:
    void sigCanvasEngineChanged();

    void sigCanvasCacheUpdated();
    void sigContinueResizeImage(qint32 w, qint32 h);

    void documentOffsetUpdateFinished();

    // emitted whenever the canvas widget thinks sketch should update
    void updateCanvasRequested(const QRect &rc);

    void sigRegionOfInterestChanged(const QRect &roi);

public Q_SLOTS:

    /// Update the entire canvas area
    void updateCanvas();

    void startResizingImage();
    void finishResizingImage(qint32 w, qint32 h);

    /// canvas rotation in degrees
    qreal rotationAngle() const;
    /// Bools indicating canvasmirroring.
    bool xAxisMirrored() const;
    bool yAxisMirrored() const;
    void slotSoftProofing(bool softProofing);
    void slotGamutCheck(bool gamutCheck);
    void slotChangeProofingConfig();
    void slotPopupPaletteRequestedZoomChange(int zoom);

    void channelSelectionChanged();

    void startUpdateInPatches(const QRect &imageRect);

    void slotTrySwitchShapeManager();

    /**
     * Called whenever the configuration settings change.
     */
    void slotConfigChanged();


private Q_SLOTS:

    /// The image projection has changed, now start an update
    /// of the canvas representation.
    void startUpdateCanvasProjection(const QRect & rc);
    void updateCanvasProjection();

    void slotBeginUpdatesBatch();
    void slotEndUpdatesBatch();
    void slotSetLodUpdatesBlocked(bool value);

    /**
     * Called whenever the view widget needs to show a different part of
     * the document
     *
     * @param documentOffset the offset in widget pixels
     */
    void documentOffsetMoved(const QPoint &documentOffset);

    void slotSelectionChanged();

    void slotDoCanvasUpdate();

    void bootstrapFinished();

    void slotUpdateRegionOfInterest();

    void slotReferenceImagesChanged();

    void slotImageColorSpaceChanged();
public:

    bool isPopupPaletteVisible() const;
    void slotShowPopupPalette(const QPoint& = QPoint(0,0));

    // interface for KisCanvasController only
    void setWrapAroundViewingMode(bool value);
    bool wrapAroundViewingMode() const;

    void setLodAllowedInCanvas(bool value);
    bool lodAllowedInCanvas() const;

    void initializeImage();

    void setFavoriteResourceManager(KisFavoriteResourceManager* favoriteResourceManager);

private:
    Q_DISABLE_COPY(KisCanvas2)

    void connectCurrentCanvas();
    void createCanvas(bool useOpenGL);
    void createQPainterCanvas();
    void createOpenGLCanvas();
    void updateCanvasWidgetImpl(const QRect &rc = QRect());
    void setCanvasWidget(KisAbstractCanvasWidget *widget);
    void resetCanvas(bool useOpenGL);
    void setDisplayProfile(const KoColorProfile *profile);

    void notifyLevelOfDetailChange();

    // Completes construction of canvas.
    // To be called by KisView in its constructor, once it has been setup enough
    // (to be defined what that means) for things KisCanvas2 expects from KisView
    // TODO: see to avoid that
    void setup();

    void initializeFpsDecoration();

private:
    friend class KisView; // calls setup()
    class KisCanvas2Private;
    KisCanvas2Private * const m_d;
};

#endif
