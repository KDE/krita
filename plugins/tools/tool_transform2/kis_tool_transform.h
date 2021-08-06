/*
 *  kis_tool_transform.h - part of Krita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_TRANSFORM_H_
#define KIS_TOOL_TRANSFORM_H_

#include <kis_icon.h>

#include <QPoint>
#include <QPointF>
#include <QVector2D>
#include <QVector3D>
#include <QButtonGroup>
#include <QPointer>

#include <QKeySequence>

#include <KoToolFactoryBase.h>

#include <kis_shape_selection.h>
#include <kis_undo_adapter.h>
#include <kis_types.h>
#include <flake/kis_node_shape.h>
#include <kis_tool.h>
#include <kis_canvas2.h>
#include <kis_action.h>

#include <KisToolPaintFactoryBase.h>


#include "tool_transform_args.h"
#include "KisToolChangesTracker.h"
#include "kis_tool_transform_config_widget.h"
#include "transform_transaction_properties.h"
#include "kis_signal_auto_connection.h"

#include "strokes/inplace_transform_stroke_strategy.h"

class QTouchEvent;
class KisTransformStrategyBase;
class KisWarpTransformStrategy;
class KisCageTransformStrategy;
class KisLiquifyTransformStrategy;
class KisFreeTransformStrategy;
class KisPerspectiveTransformStrategy;
class KisMeshTransformStrategy;


/**
 * Transform tool
 * This tool offers several modes.
 * - Free Transform mode allows the user to translate, scale, shear, rotate and
 *   apply a perspective transformation to a selection or the whole canvas.
 * - Warp mode allows the user to warp the selection of the canvas by grabbing
 *   and moving control points placed on the image. The user can either work
 *   with default control points, like a grid whose density can be modified, or
 *   place the control points manually. The modifications made on the selected
 *   pixels are applied only when the user clicks the Apply button : the
 *   semi-transparent image displayed until the user click that button is only a
 *   preview.
 * - Cage transform is similar to warp transform with control points exactly
 *   placed on the outer boundary. The user draws a boundary polygon, the
 *   vertices of which become control points.
 * - Perspective transform applies a two-point perspective transformation. The
 *   user can manipulate the corners of the selection. If the vanishing points
 *   of the resulting quadrilateral are on screen, the user can manipulate those
 *   as well.
 * - Liquify transform transforms the selection by painting motions, as if the
 *   user were fingerpainting.
 */
class KisToolTransform : public KisTool
{

    Q_OBJECT

    Q_PROPERTY(TransformToolMode transformMode READ transformMode WRITE setTransformMode NOTIFY transformModeChanged)

    Q_PROPERTY(double translateX READ translateX WRITE setTranslateX NOTIFY freeTransformChanged)
    Q_PROPERTY(double translateY READ translateY WRITE setTranslateY NOTIFY freeTransformChanged)

    Q_PROPERTY(double rotateX READ rotateX WRITE setRotateX NOTIFY freeTransformChanged)
    Q_PROPERTY(double rotateY READ rotateY WRITE setRotateY NOTIFY freeTransformChanged)
    Q_PROPERTY(double rotateZ READ rotateZ WRITE setRotateZ NOTIFY freeTransformChanged)

    Q_PROPERTY(double scaleX READ scaleX WRITE setScaleX NOTIFY freeTransformChanged)
    Q_PROPERTY(double scaleY READ scaleY WRITE setScaleY NOTIFY freeTransformChanged)

    Q_PROPERTY(double shearX READ shearX WRITE setShearX NOTIFY freeTransformChanged)
    Q_PROPERTY(double shearY READ shearY WRITE setShearY NOTIFY freeTransformChanged)

    Q_PROPERTY(WarpType warpType READ warpType WRITE setWarpType NOTIFY warpTransformChanged)
    Q_PROPERTY(double warpFlexibility READ warpFlexibility WRITE setWarpFlexibility NOTIFY warpTransformChanged)
    Q_PROPERTY(int warpPointDensity READ warpPointDensity WRITE setWarpPointDensity NOTIFY warpTransformChanged)



public:
    enum TransformToolMode {
        FreeTransformMode,
        WarpTransformMode,
        CageTransformMode,
        LiquifyTransformMode,
        PerspectiveTransformMode,
        MeshTransformMode
    };
    Q_ENUMS(TransformToolMode)

    enum WarpType {
        RigidWarpType,
        AffineWarpType,
        SimilitudeWarpType
    };
    Q_ENUMS(WarpType)

    KisToolTransform(KoCanvasBase * canvas);
    ~KisToolTransform() override;

    /**
     * @brief wantsAutoScroll
     * reimplemented from KoToolBase
     * there's an issue where autoscrolling with this tool never makes the
     * stroke end, so we return false here so that users don't get stuck with
     * the tool. See bug 362659
     * @return false
     */
    bool wantsAutoScroll() const override {
        return false;
    }

    QWidget* createOptionWidget() override;

    void mousePressEvent(KoPointerEvent *e) override;
    void mouseMoveEvent(KoPointerEvent *e) override;
    void mouseReleaseEvent(KoPointerEvent *e) override;
    void beginActionImpl(KoPointerEvent *event, bool usePrimaryAction, KisTool::AlternateAction action);
    void continueActionImpl(KoPointerEvent *event, bool usePrimaryAction, KisTool::AlternateAction action);
    void endActionImpl(KoPointerEvent *event, bool usePrimaryAction, KisTool::AlternateAction action);
    QMenu* popupActionsMenu() override;

    void activatePrimaryAction() override;
    void deactivatePrimaryAction() override;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void activateAlternateAction(AlternateAction action) override;
    void deactivateAlternateAction(AlternateAction action) override;
    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    void paint(QPainter& gc, const KoViewConverter &converter) override;

    void newActivationWithExternalSource(KisPaintDeviceSP externalSource) override;

    TransformToolMode transformMode() const;

    double translateX() const;
    double translateY() const;

    double rotateX() const;
    double rotateY() const;
    double rotateZ() const;

    double scaleX() const;
    double scaleY() const;

    double shearX() const;
    double shearY() const;

    WarpType warpType() const;
    double warpFlexibility() const;
    int warpPointDensity() const;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    // Applies the current transformation to the original paint device and commits it to the undo stack
    void applyTransform();

    void requestImageRecalculation();

    void setTransformMode( KisToolTransform::TransformToolMode newMode );

    void setTranslateX(double translateX);
    void setTranslateY(double translateY);

    void setRotateX(double rotation);
    void setRotateY(double rotation);
    void setRotateZ(double rotation);

    void setScaleX(double scaleX);
    void setScaleY(double scaleY);

    void setShearX(double shearX);
    void setShearY(double shearY);

    void setWarpType(WarpType type);
    void setWarpFlexibility(double flexibility);
    void setWarpPointDensity(int density);

protected Q_SLOTS:
    void resetCursorStyle() override;
    void slotGlobalConfigChanged();

Q_SIGNALS:
    void transformModeChanged();
    void freeTransformChanged();
    void warpTransformChanged();

public Q_SLOTS:
    void requestUndoDuringStroke() override;
    void requestRedoDuringStroke() override;
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;
    void canvasUpdateRequested();
    void cursorOutlineUpdateRequested(const QPointF &imagePos);

    // Update the widget according to m_currentArgs
    void updateOptionWidget();

    void resetRotationCenterButtonsRequested();
    void imageTooBigRequested(bool value);

private:
    QList<KisNodeSP> fetchNodesList(ToolTransformArgs::TransformMode mode, KisNodeSP root, bool isExternalSourcePresent);
    QScopedPointer<QMenu> m_contextMenu;

    void startStroke(ToolTransformArgs::TransformMode mode, bool forceReset);
    void endStroke();
    void cancelStroke();

private:
    void outlineChanged();
    // Sets the cursor according to mouse position (doesn't take shearing into account well yet)
    void setFunctionalCursor();
    // Sets m_function according to mouse position and modifier
    void setTransformFunction(QPointF mousePos, Qt::KeyboardModifiers modifiers);

    void commitChanges();

    void initTransformMode(ToolTransformArgs::TransformMode mode);
    void initGuiAfterTransformMode();

    void initThumbnailImage(KisPaintDeviceSP previewDevice);
    void updateApplyResetAvailability();

private:
    ToolTransformArgs m_currentArgs;

    // Set by newActivationWithExternalSource before starting a new stroke.
    // The source pixels for the next transform will be read from this device.
    KisPaintDeviceSP m_externalSourceForNextActivation;

    bool m_actuallyMoveWhileSelected {false}; // true <=> selection has been moved while clicked

    KisPaintDeviceSP m_selectedPortionCache;
    KisStrokeId m_strokeId;
    void *m_strokeStrategyCookie {0};
    bool m_currentlyUsingOverlayPreviewStyle {false};
    bool m_preferOverlayPreviewStyle {false};
    bool m_forceLodMode {false};


    QPainterPath m_selectionPath; // original (unscaled) selection outline, used for painting decorations

    KisToolTransformConfigWidget *m_optionsWidget {0};
    QPointer<KisCanvas2> m_canvas;

    TransformTransactionProperties m_transaction;
    KisToolChangesTracker m_changesTracker;
    KisSignalAutoConnectionsStore m_actionConnections;


    /// actions for the context click menu
    KisAction* warpAction {0};
    KisAction* meshAction {0};
    KisAction* liquifyAction {0};
    KisAction* cageAction {0};
    KisAction* freeTransformAction {0};
    KisAction* perspectiveAction {0};
    KisAction* applyTransformation {0};
    KisAction* resetTransformation {0};

    // a few extra context click options if free transform is active
    KisAction* mirrorHorizontalAction {0};
    KisAction* mirrorVericalAction {0};
    KisAction* rotateNinteyCWAction {0};
    KisAction* rotateNinteyCCWAction {0};




    /**
     * This artificial rect is used to store the image to flake
     * transformation. We check against this rect to get to know
     * whether zoom has changed.
     */
    QRectF m_refRect;

    QScopedPointer<KisWarpTransformStrategy> m_warpStrategy;
    QScopedPointer<KisCageTransformStrategy> m_cageStrategy;
    QScopedPointer<KisLiquifyTransformStrategy> m_liquifyStrategy;
    QScopedPointer<KisMeshTransformStrategy> m_meshStrategy;
    QScopedPointer<KisFreeTransformStrategy> m_freeStrategy;
    QScopedPointer<KisPerspectiveTransformStrategy> m_perspectiveStrategy;
    KisTransformStrategyBase* currentStrategy() const;

    QPainterPath m_cursorOutline;

    KisAsyncronousStrokeUpdateHelper m_asyncUpdateHelper;

private Q_SLOTS:
    void slotTrackerChangedConfig(KisToolChangesTrackerDataSP status);
    void slotUiChangedConfig(bool needsPreviewRecalculation);
    void slotApplyTransform();
    void slotResetTransform(ToolTransformArgs::TransformMode mode);
    void slotCancelTransform();
    void slotRestartTransform();
    void slotEditingFinished();

    void slotMoveDiscreteUp();
    void slotMoveDiscreteUpMore();
    void slotMoveDiscreteDown();
    void slotMoveDiscreteDownMore();
    void slotMoveDiscreteLeft();
    void slotMoveDiscreteLeftMore();
    void slotMoveDiscreteRight();
    void slotMoveDiscreteRightMore();

    void slotTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *strokeStrategyCookie);
    void slotPreviewDeviceGenerated(KisPaintDeviceSP device);

    // context menu options for updating the transform type
    // this is to help with discoverability since come people can't find the tool options
    void slotUpdateToWarpType();
    void slotUpdateToPerspectiveType();
    void slotUpdateToFreeTransformType();
    void slotUpdateToLiquifyType();
    void slotUpdateToMeshType();
    void slotUpdateToCageType();
};

class KisToolTransformFactory : public KisToolPaintFactoryBase
{
public:

    KisToolTransformFactory()
            : KisToolPaintFactoryBase("KisToolTransform") {
        setToolTip(i18n("Transform a layer or a selection"));
        setSection(ToolBoxSection::Transform);
        setIconName(koIconNameCStr("krita_tool_transform"));
        setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
        setPriority(2);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolTransformFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolTransform(canvas);
    }

    QList<QAction *> createActionsImpl() override;
};



#endif // KIS_TOOL_TRANSFORM_H_

