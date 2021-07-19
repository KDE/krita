/*
 *  kis_tool_transform.cc -- part of Krita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_transform.h"


#include <math.h>
#include <limits>

#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <QMatrix4x4>
#include <QMenu>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoPointerEvent.h>
#include <KoID.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoCompositeOp.h>

#include <kis_global.h>
#include <canvas/kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_painter.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_transaction.h>
#include <kis_selection.h>
#include <kis_filter_strategy.h>
#include <widgets/kis_cmb_idlist.h>
#include <kis_statusbar.h>
#include <kis_transform_worker.h>
#include <kis_perspectivetransform_worker.h>
#include <kis_warptransform_worker.h>
#include <kis_pixel_selection.h>
#include <kis_shape_selection.h>
#include <kis_selection_manager.h>
#include <krita_utils.h>
#include <kis_resources_snapshot.h>

#include <KoShapeTransformCommand.h>

#include "kis_action_registry.h"

#include "widgets/kis_progress_widget.h"

#include "kis_transform_utils.h"
#include "kis_warp_transform_strategy.h"
#include "kis_cage_transform_strategy.h"
#include "kis_liquify_transform_strategy.h"
#include "kis_free_transform_strategy.h"
#include "kis_perspective_transform_strategy.h"
#include "kis_mesh_transform_strategy.h"

#include "kis_transform_mask.h"
#include "kis_transform_mask_adapter.h"

#include "krita_container_utils.h"
#include "kis_layer_utils.h"
#include <KisDelayedUpdateNodeInterface.h>
#include "kis_config_notifier.h"

#include "strokes/transform_stroke_strategy.h"
#include "strokes/inplace_transform_stroke_strategy.h"

KisToolTransform::KisToolTransform(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::rotateCursor())
    , m_warpStrategy(
        new KisWarpTransformStrategy(
            dynamic_cast<KisCanvas2*>(canvas)->coordinatesConverter(),
            dynamic_cast<KisCanvas2*>(canvas)->snapGuide(),
            m_currentArgs, m_transaction))
    , m_cageStrategy(
        new KisCageTransformStrategy(
            dynamic_cast<KisCanvas2*>(canvas)->coordinatesConverter(),
            dynamic_cast<KisCanvas2*>(canvas)->snapGuide(),
            m_currentArgs, m_transaction))
    , m_liquifyStrategy(
        new KisLiquifyTransformStrategy(
            dynamic_cast<KisCanvas2*>(canvas)->coordinatesConverter(),
            m_currentArgs, m_transaction, canvas->resourceManager()))
    , m_meshStrategy(
        new KisMeshTransformStrategy(
            dynamic_cast<KisCanvas2*>(canvas)->coordinatesConverter(),
            dynamic_cast<KisCanvas2*>(canvas)->snapGuide(),
            m_currentArgs, m_transaction))
    , m_freeStrategy(
        new KisFreeTransformStrategy(
            dynamic_cast<KisCanvas2*>(canvas)->coordinatesConverter(),
            dynamic_cast<KisCanvas2*>(canvas)->snapGuide(),
            m_currentArgs, m_transaction))
    , m_perspectiveStrategy(
        new KisPerspectiveTransformStrategy(
            dynamic_cast<KisCanvas2*>(canvas)->coordinatesConverter(),
            dynamic_cast<KisCanvas2*>(canvas)->snapGuide(),
            m_currentArgs, m_transaction))
{
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    Q_ASSERT(m_canvas);

    setObjectName("tool_transform");
    useCursor(KisCursor::selectCursor());
    m_optionsWidget = 0;

    warpAction = new KisAction(i18nc("Warp Transform Tab Label", "Warp"));
    liquifyAction = new KisAction(i18nc("Liquify Transform Tab Label", "Liquify"));
    meshAction = new KisAction(i18nc("Mesh Transform Tab Label", "Mesh"));
    cageAction = new KisAction(i18nc("Cage Transform Tab Label", "Cage"));
    freeTransformAction = new KisAction(i18nc("Free Transform Tab Label", "Free"));
    perspectiveAction = new KisAction(i18nc("Perspective Transform Tab Label", "Perspective"));

    // extra actions for free transform that are in the tool options
    mirrorHorizontalAction = new KisAction(i18n("Mirror Horizontal"));
    mirrorVericalAction = new KisAction(i18n("Mirror Vertical"));
    rotateNinteyCWAction = new KisAction(i18n("Rotate 90 degrees Clockwise"));
    rotateNinteyCCWAction = new KisAction(i18n("Rotate 90 degrees CounterClockwise"));

    applyTransformation = new KisAction(i18n("Apply"));
    resetTransformation = new KisAction(i18n("Reset"));

    m_contextMenu.reset(new QMenu());

    connect(m_warpStrategy.data(), SIGNAL(requestCanvasUpdate()), SLOT(canvasUpdateRequested()));
    connect(m_warpStrategy.data(), SIGNAL(requestImageRecalculation()), SLOT(requestImageRecalculation()));
    connect(m_cageStrategy.data(), SIGNAL(requestCanvasUpdate()), SLOT(canvasUpdateRequested()));
    connect(m_cageStrategy.data(), SIGNAL(requestImageRecalculation()), SLOT(requestImageRecalculation()));
    connect(m_liquifyStrategy.data(), SIGNAL(requestCanvasUpdate()), SLOT(canvasUpdateRequested()));
    connect(m_liquifyStrategy.data(), SIGNAL(requestCursorOutlineUpdate(QPointF)), SLOT(cursorOutlineUpdateRequested(QPointF)));
    connect(m_liquifyStrategy.data(), SIGNAL(requestUpdateOptionWidget()), SLOT(updateOptionWidget()));
    connect(m_liquifyStrategy.data(), SIGNAL(requestImageRecalculation()), SLOT(requestImageRecalculation()));
    connect(m_freeStrategy.data(), SIGNAL(requestCanvasUpdate()), SLOT(canvasUpdateRequested()));
    connect(m_freeStrategy.data(), SIGNAL(requestResetRotationCenterButtons()), SLOT(resetRotationCenterButtonsRequested()));
    connect(m_freeStrategy.data(), SIGNAL(requestShowImageTooBig(bool)), SLOT(imageTooBigRequested(bool)));
    connect(m_freeStrategy.data(), SIGNAL(requestImageRecalculation()), SLOT(requestImageRecalculation()));
    connect(m_perspectiveStrategy.data(), SIGNAL(requestCanvasUpdate()), SLOT(canvasUpdateRequested()));
    connect(m_perspectiveStrategy.data(), SIGNAL(requestShowImageTooBig(bool)), SLOT(imageTooBigRequested(bool)));
    connect(m_perspectiveStrategy.data(), SIGNAL(requestImageRecalculation()), SLOT(requestImageRecalculation()));
    connect(m_meshStrategy.data(), SIGNAL(requestCanvasUpdate()), SLOT(canvasUpdateRequested()));
    connect(m_meshStrategy.data(), SIGNAL(requestImageRecalculation()), SLOT(requestImageRecalculation()));

    connect(&m_changesTracker, SIGNAL(sigConfigChanged(KisToolChangesTrackerDataSP)),
            this, SLOT(slotTrackerChangedConfig(KisToolChangesTrackerDataSP)));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotGlobalConfigChanged()));
}

KisToolTransform::~KisToolTransform()
{
    cancelStroke();
}

void KisToolTransform::outlineChanged()
{
    emit freeTransformChanged();
    m_canvas->updateCanvas();
}

void KisToolTransform::canvasUpdateRequested()
{
    m_canvas->updateCanvas();
}

void KisToolTransform::resetCursorStyle()
{
    setFunctionalCursor();
}

void KisToolTransform::slotGlobalConfigChanged()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(toolId());
    m_preferOverlayPreviewStyle = group.readEntry("useOverlayPreviewStyle", false);
    m_forceLodMode = group.readEntry("forceLodMode", true);
}

void KisToolTransform::resetRotationCenterButtonsRequested()
{
    if (!m_optionsWidget) return;
    m_optionsWidget->resetRotationCenterButtons();
}

void KisToolTransform::imageTooBigRequested(bool value)
{
    if (!m_optionsWidget) return;
    m_optionsWidget->setTooBigLabelVisible(value);
}

KisTransformStrategyBase* KisToolTransform::currentStrategy() const
{
    if (m_currentArgs.mode() == ToolTransformArgs::FREE_TRANSFORM) {
        return m_freeStrategy.data();
    } else if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
        return m_warpStrategy.data();
    } else if (m_currentArgs.mode() == ToolTransformArgs::CAGE) {
        return m_cageStrategy.data();
    } else if (m_currentArgs.mode() == ToolTransformArgs::LIQUIFY) {
        return m_liquifyStrategy.data();
    } else if (m_currentArgs.mode() == ToolTransformArgs::MESH) {
        return m_meshStrategy.data();
    } else /* if (m_currentArgs.mode() == ToolTransformArgs::PERSPECTIVE_4POINT) */ {
        return m_perspectiveStrategy.data();
    }
}

void KisToolTransform::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (!m_strokeId || !m_transaction.rootNode()) return;

    QRectF newRefRect = KisTransformUtils::imageToFlake(m_canvas->coordinatesConverter(), QRectF(0.0,0.0,1.0,1.0));
    if (m_refRect != newRefRect) {
        m_refRect = newRefRect;
        currentStrategy()->externalConfigChanged();
    }
    currentStrategy()->paint(gc);


    if (!m_cursorOutline.isEmpty()) {
        QPainterPath mappedOutline =
            KisTransformUtils::imageToFlakeTransform(
                m_canvas->coordinatesConverter()).map(m_cursorOutline);
        paintToolOutline(&gc, mappedOutline);
    }
}

void KisToolTransform::setFunctionalCursor()
{
    if (overrideCursorIfNotEditable()) {
        return;
    }

    if (!m_strokeId) {
        useCursor(KisCursor::pointingHandCursor());
    } else if (m_strokeId && !m_transaction.rootNode()) {
        // we are in the middle of stroke initialization
        useCursor(KisCursor::waitCursor());
    } else {
        useCursor(currentStrategy()->getCurrentCursor());
    }
}

void KisToolTransform::cursorOutlineUpdateRequested(const QPointF &imagePos)
{
    QRect canvasUpdateRect;

    if (!m_cursorOutline.isEmpty()) {
        canvasUpdateRect = m_canvas->coordinatesConverter()->
            imageToDocument(m_cursorOutline.boundingRect()).toAlignedRect();
    }

    m_cursorOutline = currentStrategy()->
        getCursorOutline().translated(imagePos);

    if (!m_cursorOutline.isEmpty()) {
        canvasUpdateRect |=
            m_canvas->coordinatesConverter()->
            imageToDocument(m_cursorOutline.boundingRect()).toAlignedRect();
    }

    if (!canvasUpdateRect.isEmpty()) {
        // grow rect a bit to follow interpolation fuzziness
        canvasUpdateRect = kisGrowRect(canvasUpdateRect, 2);
        m_canvas->updateCanvas(canvasUpdateRect);
    }
}

void KisToolTransform::beginActionImpl(KoPointerEvent *event, bool usePrimaryAction, KisTool::AlternateAction action)
{
    if (!nodeEditable()) {
        event->ignore();
        return;
    }

    if (!m_strokeId) {
        startStroke(m_currentArgs.mode(), action == KisTool::ChangeSize);
    } else if (m_transaction.rootNode()) {
        bool result = false;

        if (usePrimaryAction) {
            result = currentStrategy()->beginPrimaryAction(event);
        } else {
            result = currentStrategy()->beginAlternateAction(event, action);
        }

        if (result) {
            setMode(KisTool::PAINT_MODE);
        }
    }

    m_actuallyMoveWhileSelected = false;

    outlineChanged();
}

void KisToolTransform::continueActionImpl(KoPointerEvent *event, bool usePrimaryAction, KisTool::AlternateAction action)
{
    if (mode() != KisTool::PAINT_MODE) return;
    if (!m_transaction.rootNode()) return;

    m_actuallyMoveWhileSelected = true;

    if (usePrimaryAction) {
        currentStrategy()->continuePrimaryAction(event);
    } else {
        currentStrategy()->continueAlternateAction(event, action);
    }

    updateOptionWidget();
    outlineChanged();
}

void KisToolTransform::endActionImpl(KoPointerEvent *event, bool usePrimaryAction, KisTool::AlternateAction action)
{
    if (mode() != KisTool::PAINT_MODE) return;

    setMode(KisTool::HOVER_MODE);

    if (m_actuallyMoveWhileSelected ||
        currentStrategy()->acceptsClicks()) {

        bool result = false;

        if (usePrimaryAction) {
            result = currentStrategy()->endPrimaryAction(event);
        } else {
            result = currentStrategy()->endAlternateAction(event, action);
        }

        if (result) {
            commitChanges();
        }

        outlineChanged();
    }

    updateOptionWidget();
    updateApplyResetAvailability();
}

QMenu* KisToolTransform::popupActionsMenu()
{
    if (m_contextMenu) {
        m_contextMenu->clear();

        m_contextMenu->addSection(i18n("Transform Tool Actions"));
        // add a quick switch to different transform types
        m_contextMenu->addAction(freeTransformAction);
        m_contextMenu->addAction(perspectiveAction);
        m_contextMenu->addAction(warpAction);
        m_contextMenu->addAction(cageAction);
        m_contextMenu->addAction(liquifyAction);
        m_contextMenu->addAction(meshAction);

        // extra options if free transform is selected
        if (transformMode() == FreeTransformMode) {
            m_contextMenu->addSeparator();
            m_contextMenu->addAction(mirrorHorizontalAction);
            m_contextMenu->addAction(mirrorVericalAction);
            m_contextMenu->addAction(rotateNinteyCWAction);
            m_contextMenu->addAction(rotateNinteyCCWAction);
        }

        m_contextMenu->addSeparator();
        m_contextMenu->addAction(applyTransformation);
        m_contextMenu->addAction(resetTransformation);
    }

    return m_contextMenu.data();
}

void KisToolTransform::beginPrimaryAction(KoPointerEvent *event)
{
    beginActionImpl(event, true, KisTool::NONE);
}

void KisToolTransform::continuePrimaryAction(KoPointerEvent *event)
{
    continueActionImpl(event, true, KisTool::NONE);
}

void KisToolTransform::endPrimaryAction(KoPointerEvent *event)
{
    endActionImpl(event, true, KisTool::NONE);
}

void KisToolTransform::activatePrimaryAction()
{
    currentStrategy()->activatePrimaryAction();
    setFunctionalCursor();
}

void KisToolTransform::deactivatePrimaryAction()
{
    currentStrategy()->deactivatePrimaryAction();
}

void KisToolTransform::activateAlternateAction(AlternateAction action)
{
    currentStrategy()->activateAlternateAction(action);
    setFunctionalCursor();
}

void KisToolTransform::deactivateAlternateAction(AlternateAction action)
{
    currentStrategy()->deactivateAlternateAction(action);
}

void KisToolTransform::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    beginActionImpl(event, false, action);
}

void KisToolTransform::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    continueActionImpl(event, false, action);
}

void KisToolTransform::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    endActionImpl(event, false, action);
}

void KisToolTransform::mousePressEvent(KoPointerEvent *event)
{
    KisTool::mousePressEvent(event);
}

void KisToolTransform::mouseMoveEvent(KoPointerEvent *event)
{
    QPointF mousePos = m_canvas->coordinatesConverter()->documentToImage(event->point);

    cursorOutlineUpdateRequested(mousePos);

    if (this->mode() != KisTool::PAINT_MODE) {
        currentStrategy()->hoverActionCommon(event);
        setFunctionalCursor();
        KisTool::mouseMoveEvent(event);
        return;
    }
}

void KisToolTransform::mouseReleaseEvent(KoPointerEvent *event)
{
    KisTool::mouseReleaseEvent(event);
}

void KisToolTransform::applyTransform()
{
    slotApplyTransform();
}

KisToolTransform::TransformToolMode KisToolTransform::transformMode() const
{
    TransformToolMode mode = FreeTransformMode;

    switch (m_currentArgs.mode())
    {
    case ToolTransformArgs::FREE_TRANSFORM:
        mode = FreeTransformMode;
        break;
    case ToolTransformArgs::WARP:
        mode = WarpTransformMode;
        break;
    case ToolTransformArgs::CAGE:
        mode = CageTransformMode;
        break;
    case ToolTransformArgs::LIQUIFY:
        mode = LiquifyTransformMode;
        break;
    case ToolTransformArgs::PERSPECTIVE_4POINT:
        mode = PerspectiveTransformMode;
        break;
    case ToolTransformArgs::MESH:
        mode = MeshTransformMode;
        break;
    default:
        KIS_ASSERT_RECOVER_NOOP(0 && "unexpected transform mode");
    }

    return mode;
}

double KisToolTransform::translateX() const
{
    return m_currentArgs.transformedCenter().x();
}

double KisToolTransform::translateY() const
{
    return m_currentArgs.transformedCenter().y();
}

double KisToolTransform::rotateX() const
{
    return m_currentArgs.aX();
}

double KisToolTransform::rotateY() const
{
    return m_currentArgs.aY();
}

double KisToolTransform::rotateZ() const
{
    return m_currentArgs.aZ();
}

double KisToolTransform::scaleX() const
{
    return m_currentArgs.scaleX();
}

double KisToolTransform::scaleY() const
{
    return m_currentArgs.scaleY();
}

double KisToolTransform::shearX() const
{
    return m_currentArgs.shearX();
}

double KisToolTransform::shearY() const
{
    return m_currentArgs.shearY();
}

KisToolTransform::WarpType KisToolTransform::warpType() const
{
    switch(m_currentArgs.warpType()) {
    case KisWarpTransformWorker::AFFINE_TRANSFORM:
        return AffineWarpType;
    case KisWarpTransformWorker::RIGID_TRANSFORM:
        return RigidWarpType;
    case KisWarpTransformWorker::SIMILITUDE_TRANSFORM:
        return SimilitudeWarpType;
    default:
        return RigidWarpType;
    }
}

double KisToolTransform::warpFlexibility() const
{
    return m_currentArgs.alpha();
}

int KisToolTransform::warpPointDensity() const
{
    return m_currentArgs.numPoints();
}

void KisToolTransform::setTransformMode(KisToolTransform::TransformToolMode newMode)
{
    ToolTransformArgs::TransformMode mode = ToolTransformArgs::FREE_TRANSFORM;

    switch (newMode) {
    case FreeTransformMode:
        mode = ToolTransformArgs::FREE_TRANSFORM;
        break;
    case WarpTransformMode:
        mode = ToolTransformArgs::WARP;
        break;
    case CageTransformMode:
        mode = ToolTransformArgs::CAGE;
        break;
    case LiquifyTransformMode:
        mode = ToolTransformArgs::LIQUIFY;
        break;
    case PerspectiveTransformMode:
        mode = ToolTransformArgs::PERSPECTIVE_4POINT;
        break;
    case MeshTransformMode:
        mode = ToolTransformArgs::MESH;
        break;
    default:
        KIS_ASSERT_RECOVER_NOOP(0 && "unexpected transform mode");
    }

    if( mode != m_currentArgs.mode() ) {
        if( newMode == FreeTransformMode ) {
            m_optionsWidget->slotSetFreeTransformModeButtonClicked( true );
        } else if( newMode == WarpTransformMode ) {
            m_optionsWidget->slotSetWarpModeButtonClicked( true );
        } else if( newMode == CageTransformMode ) {
            m_optionsWidget->slotSetCageModeButtonClicked( true );
        } else if( newMode == LiquifyTransformMode ) {
            m_optionsWidget->slotSetLiquifyModeButtonClicked( true );
        } else if( newMode == PerspectiveTransformMode ) {
            m_optionsWidget->slotSetPerspectiveModeButtonClicked( true );
        } else if( newMode == MeshTransformMode ) {
            m_optionsWidget->slotSetMeshModeButtonClicked( true );
        }

        emit transformModeChanged();
    }
}

void KisToolTransform::setRotateX( double rotation )
{
    m_currentArgs.setAX( rotation );
}

void KisToolTransform::setRotateY( double rotation )
{
    m_currentArgs.setAY( rotation );
}

void KisToolTransform::setRotateZ( double rotation )
{
    m_currentArgs.setAZ( rotation );
}

void KisToolTransform::setWarpType( KisToolTransform::WarpType type )
{
    switch( type ) {
    case RigidWarpType:
        m_currentArgs.setWarpType(KisWarpTransformWorker::RIGID_TRANSFORM);
        break;
    case AffineWarpType:
        m_currentArgs.setWarpType(KisWarpTransformWorker::AFFINE_TRANSFORM);
        break;
    case SimilitudeWarpType:
        m_currentArgs.setWarpType(KisWarpTransformWorker::SIMILITUDE_TRANSFORM);
        break;
    default:
        break;
    }
}

void KisToolTransform::setWarpFlexibility( double flexibility )
{
    m_currentArgs.setAlpha( flexibility );
}

void KisToolTransform::setWarpPointDensity( int density )
{
    m_optionsWidget->slotSetWarpDensity(density);
}

void KisToolTransform::initTransformMode(ToolTransformArgs::TransformMode mode)
{
    m_currentArgs = KisTransformUtils::resetArgsForMode(mode, m_currentArgs.filterId(), m_transaction, m_currentArgs.externalSource());
    initGuiAfterTransformMode();
}

void KisToolTransform::initGuiAfterTransformMode()
{
    currentStrategy()->externalConfigChanged();
    outlineChanged();
    updateOptionWidget();
    updateApplyResetAvailability();
    setFunctionalCursor();
}

void KisToolTransform::initThumbnailImage(KisPaintDeviceSP previewDevice)
{
    QImage origImg;
    m_selectedPortionCache = previewDevice;

    QTransform thumbToImageTransform;

    const int maxSize = 2000;

    QRect srcRect(m_transaction.originalRect().toAlignedRect());
    int x, y, w, h;
    srcRect.getRect(&x, &y, &w, &h);

    if (m_selectedPortionCache) {
        if (w > maxSize || h > maxSize) {
            qreal scale = qreal(maxSize) / (w > h ? w : h);
            QTransform scaleTransform = QTransform::fromScale(scale, scale);

            QRect thumbRect = scaleTransform.mapRect(m_transaction.originalRect()).toAlignedRect();

            origImg = m_selectedPortionCache->
                    createThumbnail(thumbRect.width(),
                                    thumbRect.height(),
                                    srcRect, 1,
                                    KoColorConversionTransformation::internalRenderingIntent(),
                                    KoColorConversionTransformation::internalConversionFlags());
            thumbToImageTransform = scaleTransform.inverted();

        } else {
            origImg = m_selectedPortionCache->convertToQImage(0, x, y, w, h,
                                                              KoColorConversionTransformation::internalRenderingIntent(),
                                                              KoColorConversionTransformation::internalConversionFlags());
            thumbToImageTransform = QTransform();
        }
    }

    // init both strokes since the thumbnail is initialized only once
    // during the stroke
    m_freeStrategy->setThumbnailImage(origImg, thumbToImageTransform);
    m_perspectiveStrategy->setThumbnailImage(origImg, thumbToImageTransform);
    m_warpStrategy->setThumbnailImage(origImg, thumbToImageTransform);
    m_cageStrategy->setThumbnailImage(origImg, thumbToImageTransform);
    m_liquifyStrategy->setThumbnailImage(origImg, thumbToImageTransform);
    m_meshStrategy->setThumbnailImage(origImg, thumbToImageTransform);
}

void KisToolTransform::newActivationWithExternalSource(KisPaintDeviceSP externalSource)
{
    m_externalSourceForNextActivation = externalSource;
    if (isActive()) {
        QSet<KoShape*> dummy;
        deactivate();
        activate(dummy);
    } else {
        KoToolManager::instance()->switchToolRequested("KisToolTransform");
    }
}

void KisToolTransform::activate(const QSet<KoShape*> &shapes)
{
    KisTool::activate(shapes);

    /// we cannot initialize the setting in the constructor, because
    /// factory() is not yet initialized, so we cannot get toolId()
    slotGlobalConfigChanged();

    m_actionConnections.addConnection(action("movetool-move-up"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteUp()));
    m_actionConnections.addConnection(action("movetool-move-up-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteUpMore()));
    m_actionConnections.addConnection(action("movetool-move-down"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteDown()));
    m_actionConnections.addConnection(action("movetool-move-down-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteDownMore()));
    m_actionConnections.addConnection(action("movetool-move-left"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteLeft()));
    m_actionConnections.addConnection(action("movetool-move-left-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteLeftMore()));
    m_actionConnections.addConnection(action("movetool-move-right"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteRight()));
    m_actionConnections.addConnection(action("movetool-move-right-more"), SIGNAL(triggered(bool)),
                                      this, SLOT(slotMoveDiscreteRightMore()));

    if (currentNode()) {
        m_transaction = TransformTransactionProperties(QRectF(), &m_currentArgs, KisNodeSP(), {});
    }

    startStroke(ToolTransformArgs::FREE_TRANSFORM, false);
}

void KisToolTransform::deactivate()
{
    endStroke();
    m_canvas->updateCanvas();
    m_actionConnections.clear();
    KisTool::deactivate();
}

void KisToolTransform::requestUndoDuringStroke()
{
    if (!m_strokeId || !m_transaction.rootNode()) return;

    if (m_changesTracker.isEmpty(true)) {
        cancelStroke();
    } else {
        m_changesTracker.requestUndo();
    }
}

void KisToolTransform::requestRedoDuringStroke()
{
    if (!m_strokeId || !m_transaction.rootNode()) return;

    m_changesTracker.requestRedo();
}

void KisToolTransform::requestStrokeEnd()
{
    endStroke();
}

void KisToolTransform::requestStrokeCancellation()
{
    if (!m_transaction.rootNode() || m_currentArgs.isIdentity()) {
        cancelStroke();
    } else {
        slotCancelTransform();
    }
}

void KisToolTransform::requestImageRecalculation()
{
    if (!m_currentlyUsingOverlayPreviewStyle && m_strokeId && m_transaction.rootNode()) {
        image()->addJob(
            m_strokeId,
            new InplaceTransformStrokeStrategy::UpdateTransformData(
                m_currentArgs,
                InplaceTransformStrokeStrategy::UpdateTransformData::PAINT_DEVICE));
    }
}

void KisToolTransform::startStroke(ToolTransformArgs::TransformMode mode, bool forceReset)
{
    Q_ASSERT(!m_strokeId);

    KisPaintDeviceSP externalSource = m_externalSourceForNextActivation;
    m_externalSourceForNextActivation.clear();

    // set up and null checks before we do anything
    KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());
    KisNodeSP currentNode = resources->currentNode();
    if (!currentNode || !currentNode->isEditable()) return;

    m_transaction = TransformTransactionProperties(QRectF(), &m_currentArgs, KisNodeSP(), {});
    m_currentArgs = ToolTransformArgs();

    // some layer types cannot be transformed. Give a message and return if a user tries it
    if (currentNode->inherits("KisColorizeMask") ||
        currentNode->inherits("KisFileLayer") ||
        currentNode->inherits("KisCloneLayer")) {

        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());

        if(currentNode->inherits("KisColorizeMask")){
            kisCanvas->viewManager()->
                showFloatingMessage(
                    i18nc("floating message in transformation tool",
                          "Layer type cannot use the transform tool"),
                    koIcon("object-locked"), 4000, KisFloatingMessage::High);
        }
        else{
            kisCanvas->viewManager()->
                showFloatingMessage(
                    i18nc("floating message in transformation tool",
                          "Layer type cannot use the transform tool. Use transform mask instead."),
                    koIcon("object-locked"), 4000, KisFloatingMessage::High);
        }
        return;
    }

    KisNodeSP impossibleMask =
        KisLayerUtils::recursiveFindNode(currentNode,
        [currentNode] (KisNodeSP node) {
            // we can process transform masks of the first level
            if (node == currentNode || node->parent() == currentNode) return false;

            return node->inherits("KisTransformMask") && node->visible(true);
        });

    if (impossibleMask) {
        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in transformation tool",
                      "Layer has children with transform masks. Please disable them before doing transformation."),
                koIcon("object-locked"), 8000, KisFloatingMessage::High);
        return;
    }


    KisSelectionSP selection = resources->activeSelection();

    /**
     * When working with transform mask, selections are not
     * taken into account.
     */
    if (selection && dynamic_cast<KisTransformMask*>(currentNode.data())) {
        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in transformation tool",
                      "Selections are not used when editing transform masks "),
                QIcon(), 4000, KisFloatingMessage::Low);

        selection = 0;
    }

    // Overlay preview is never used when transforming an externally provided image
    m_currentlyUsingOverlayPreviewStyle = m_preferOverlayPreviewStyle && !externalSource;

    KisStrokeStrategy *strategy = 0;

    if (m_currentlyUsingOverlayPreviewStyle) {
        TransformStrokeStrategy *transformStrategy = new TransformStrokeStrategy(mode, m_currentArgs.filterId(), forceReset, currentNode, selection, image().data(), image().data());
        connect(transformStrategy, SIGNAL(sigPreviewDeviceReady(KisPaintDeviceSP)), SLOT(slotPreviewDeviceGenerated(KisPaintDeviceSP)));
        connect(transformStrategy, SIGNAL(sigTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void*)), SLOT(slotTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void*)));
        strategy = transformStrategy;

        // save unique identifier of the stroke so we could
        // recognize it when sigTransactionGenerated() is
        // received (theoretically, the user can start two
        // strokes at the same time, if he is quick enough)
        m_strokeStrategyCookie = transformStrategy;

    } else {
        InplaceTransformStrokeStrategy *transformStrategy = new InplaceTransformStrokeStrategy(mode, m_currentArgs.filterId(), forceReset, currentNode, selection, externalSource, image().data(), image().data(), image()->root(), m_forceLodMode);
        connect(transformStrategy, SIGNAL(sigTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void*)), SLOT(slotTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void*)));
        strategy = transformStrategy;

        // save unique identifier of the stroke so we could
        // recognize it when sigTransactionGenerated() is
        // received (theoretically, the user can start two
        // strokes at the same time, if he is quick enough)
        m_strokeStrategyCookie = transformStrategy;
    }

    m_strokeId = image()->startStroke(strategy);


    KIS_SAFE_ASSERT_RECOVER_NOOP(m_changesTracker.isEmpty(true));
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_changesTracker.isEmpty(false));

    slotPreviewDeviceGenerated(0);
}

void KisToolTransform::endStroke()
{
    if (!m_strokeId) return;

    if (m_currentlyUsingOverlayPreviewStyle &&
        m_transaction.rootNode() &&
        !m_currentArgs.isUnchanging()) {

        image()->addJob(m_strokeId,
                        new TransformStrokeStrategy::TransformAllData(m_currentArgs));
    }

    if (m_asyncUpdateHelper.isActive()) {
        m_asyncUpdateHelper.endUpdateStream();
    }

    image()->endStroke(m_strokeId);

    m_strokeStrategyCookie = 0;
    m_strokeId.clear();
    m_changesTracker.reset();
    m_transaction = TransformTransactionProperties(QRectF(), &m_currentArgs, KisNodeSP(), {});
    outlineChanged();
}

void KisToolTransform::slotTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *strokeStrategyCookie)
{
    if (!m_strokeId || strokeStrategyCookie != m_strokeStrategyCookie) return;

    if (transaction.transformedNodes().isEmpty() ||
        transaction.originalRect().isEmpty()) {

        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        KIS_ASSERT(kisCanvas);
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in transformation tool",
                      "Cannot transform empty layer "),
                QIcon(), 1000, KisFloatingMessage::Medium);

        cancelStroke();
        return;
    }

    m_transaction = transaction;
    m_currentArgs = args;
    m_transaction.setCurrentConfigLocation(&m_currentArgs);

    if (!m_currentlyUsingOverlayPreviewStyle) {
        m_asyncUpdateHelper.startUpdateStream(image().data(), m_strokeId);
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(m_changesTracker.isEmpty(true));
    commitChanges();

    initGuiAfterTransformMode();

    if (m_transaction.hasInvisibleNodes()) {
        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in transformation tool",
                      "Invisible sublayers will also be transformed. Lock layers if you do not want them to be transformed "),
                QIcon(), 4000, KisFloatingMessage::Low);
    }
}

void KisToolTransform::slotPreviewDeviceGenerated(KisPaintDeviceSP device)
{
    if (device && device->exactBounds().isEmpty()) {
        KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        kisCanvas->viewManager()->
            showFloatingMessage(
                i18nc("floating message in transformation tool",
                      "Cannot transform empty layer "),
                QIcon(), 1000, KisFloatingMessage::Medium);

        cancelStroke();
    } else {
        initThumbnailImage(device);
        initGuiAfterTransformMode();
    }
}

void KisToolTransform::cancelStroke()
{
    if (!m_strokeId) return;

    if (m_asyncUpdateHelper.isActive()) {
        m_asyncUpdateHelper.cancelUpdateStream();
    }

    image()->cancelStroke(m_strokeId);
    m_strokeStrategyCookie = 0;
    m_strokeId.clear();
    m_changesTracker.reset();
    m_transaction = TransformTransactionProperties(QRectF(), &m_currentArgs, KisNodeSP(), {});
    outlineChanged();
}

void KisToolTransform::commitChanges()
{
    if (!m_strokeId || !m_transaction.rootNode()) return;

    m_changesTracker.commitConfig(toQShared(m_currentArgs.clone()));
}

void KisToolTransform::slotTrackerChangedConfig(KisToolChangesTrackerDataSP status)
{
    const ToolTransformArgs *newArgs = dynamic_cast<const ToolTransformArgs*>(status.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(newArgs);

    *m_transaction.currentConfig() = *newArgs;

    slotUiChangedConfig(true);
    updateOptionWidget();
}

QList<KisNodeSP> KisToolTransform::fetchNodesList(ToolTransformArgs::TransformMode mode, KisNodeSP root, bool isExternalSourcePresent)
{
    QList<KisNodeSP> result;

    auto fetchFunc =
        [&result, mode, root] (KisNodeSP node) {
        if (node->isEditable(node == root) &&
                (!node->inherits("KisShapeLayer") || mode == ToolTransformArgs::FREE_TRANSFORM) &&
                !node->inherits("KisFileLayer") &&
                (!node->inherits("KisTransformMask") || node == root)) {

                result << node;
            }
    };

    if (isExternalSourcePresent) {
        fetchFunc(root);
    } else {
        KisLayerUtils::recursiveApplyNodes(root, fetchFunc);
    }

    return result;
}

QWidget* KisToolTransform::createOptionWidget()
{
    if (!m_canvas) return 0;
     
    m_optionsWidget = new KisToolTransformConfigWidget(&m_transaction, m_canvas, 0);
    Q_CHECK_PTR(m_optionsWidget);
    m_optionsWidget->setObjectName(toolId() + " option widget");

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(m_optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    m_optionsWidget->layout()->addWidget(specialSpacer);


    connect(m_optionsWidget, SIGNAL(sigConfigChanged(bool)),
            this, SLOT(slotUiChangedConfig(bool)));

    connect(m_optionsWidget, SIGNAL(sigApplyTransform()),
            this, SLOT(slotApplyTransform()));

    connect(m_optionsWidget, SIGNAL(sigResetTransform(ToolTransformArgs::TransformMode)),
            this, SLOT(slotResetTransform(ToolTransformArgs::TransformMode)));

    connect(m_optionsWidget, SIGNAL(sigCancelTransform()),
            this, SLOT(slotCancelTransform()));

    connect(m_optionsWidget, SIGNAL(sigRestartTransform()),
            this, SLOT(slotRestartTransform()));

    connect(m_optionsWidget, SIGNAL(sigEditingFinished()),
            this, SLOT(slotEditingFinished()));


    connect(mirrorHorizontalAction, SIGNAL(triggered(bool)), m_optionsWidget, SLOT(slotFlipX()));
    connect(mirrorVericalAction, SIGNAL(triggered(bool)), m_optionsWidget, SLOT(slotFlipY()));
    connect(rotateNinteyCWAction, SIGNAL(triggered(bool)), m_optionsWidget, SLOT(slotRotateCW()));
    connect(rotateNinteyCCWAction, SIGNAL(triggered(bool)), m_optionsWidget, SLOT(slotRotateCCW()));


    connect(warpAction, SIGNAL(triggered(bool)), this, SLOT(slotUpdateToWarpType()));
    connect(perspectiveAction, SIGNAL(triggered(bool)), this, SLOT(slotUpdateToPerspectiveType()));
    connect(freeTransformAction, SIGNAL(triggered(bool)), this, SLOT(slotUpdateToFreeTransformType()));
    connect(liquifyAction, SIGNAL(triggered(bool)), this, SLOT(slotUpdateToLiquifyType()));
    connect(meshAction, SIGNAL(triggered(bool)), this, SLOT(slotUpdateToMeshType()));
    connect(cageAction, SIGNAL(triggered(bool)), this, SLOT(slotUpdateToCageType()));

    connect(applyTransformation, SIGNAL(triggered(bool)), this, SLOT(slotApplyTransform()));
    connect(resetTransformation, SIGNAL(triggered(bool)), this, SLOT(slotCancelTransform()));


    updateOptionWidget();

    return m_optionsWidget;
}

void KisToolTransform::updateOptionWidget()
{    
    if (!m_optionsWidget) return;

    if (!currentNode()) {
        m_optionsWidget->setEnabled(false);
        return;
    }
    else {
        m_optionsWidget->setEnabled(true);
        m_optionsWidget->updateConfig(m_currentArgs);
    }
}

void KisToolTransform::updateApplyResetAvailability()
{
    if (m_optionsWidget) {
        m_optionsWidget->setApplyResetDisabled(m_currentArgs.isIdentity());
    }
}

void KisToolTransform::slotUiChangedConfig(bool needsPreviewRecalculation)
{
    if (mode() == KisTool::PAINT_MODE) return;

    if (needsPreviewRecalculation) {
        currentStrategy()->externalConfigChanged();
    }

    if (m_currentArgs.mode() == ToolTransformArgs::LIQUIFY) {
        m_currentArgs.saveLiquifyTransformMode();
    }

    outlineChanged();
    updateApplyResetAvailability();
}

void KisToolTransform::slotApplyTransform()
{
    QApplication::setOverrideCursor(KisCursor::waitCursor());
    endStroke();
    QApplication::restoreOverrideCursor();
}

void KisToolTransform::slotResetTransform(ToolTransformArgs::TransformMode mode)
{
    ToolTransformArgs *config = m_transaction.currentConfig();
    const ToolTransformArgs::TransformMode previousMode = config->mode();
    config->setMode(mode);

    if (mode == ToolTransformArgs::WARP) {
        config->setWarpCalculation(KisWarpTransformWorker::WarpCalculation::GRID);
    }

    if (!m_strokeId || !m_transaction.rootNode()) return;

    if (m_currentArgs.continuedTransform()) {
        ToolTransformArgs::TransformMode savedMode = m_currentArgs.mode();

        /**
         * Our reset transform button can be used for two purposes:
         *
         * 1) Reset current transform to the initial one, which was
         *    loaded from the previous user action.
         *
         * 2) Reset transform frame to infinity when the frame is unchanged
         */

        const bool transformDiffers = !m_currentArgs.continuedTransform()->isSameMode(m_currentArgs);

        if (transformDiffers &&
            m_currentArgs.continuedTransform()->mode() == savedMode) {

            m_currentArgs.restoreContinuedState();
            initGuiAfterTransformMode();
            slotEditingFinished();

        } else {
            cancelStroke();
            startStroke(savedMode, true);

            KIS_ASSERT_RECOVER_NOOP(!m_currentArgs.continuedTransform());
        }
    } else {
        if (!KisTransformUtils::shouldRestartStrokeOnModeChange(previousMode,
                                                                m_currentArgs.mode(),
                                                                m_transaction.transformedNodes())) {
            initTransformMode(m_currentArgs.mode());
            slotEditingFinished();

        } else {
            cancelStroke();
            startStroke(m_currentArgs.mode(), true);

        }
    }
}

void KisToolTransform::slotCancelTransform()
{
    slotResetTransform(m_transaction.currentConfig()->mode());
}

void KisToolTransform::slotRestartTransform()
{
    if (!m_strokeId || !m_transaction.rootNode()) return;

    KisNodeSP root = m_transaction.rootNode();
    KIS_ASSERT_RECOVER_RETURN(root); // the stroke is guaranteed to be started by an 'if' above

    ToolTransformArgs savedArgs(m_currentArgs);
    cancelStroke();
    startStroke(savedArgs.mode(), true);
}

void KisToolTransform::slotEditingFinished()
{
    commitChanges();
}

void KisToolTransform::slotMoveDiscreteUp()
{
    setTranslateY(translateY()-1.0);
}

void KisToolTransform::slotMoveDiscreteUpMore()
{
    setTranslateY(translateY()-10.0);
}

void KisToolTransform::slotMoveDiscreteDown()
{
    setTranslateY(translateY()+1.0);
}

void KisToolTransform::slotMoveDiscreteDownMore()
{
    setTranslateY(translateY()+10.0);
}

void KisToolTransform::slotMoveDiscreteLeft()
{
    setTranslateX(translateX()-1.0);
}

void KisToolTransform::slotMoveDiscreteLeftMore()
{
    setTranslateX(translateX()-10.0);
}

void KisToolTransform::slotMoveDiscreteRight()
{
    setTranslateX(translateX()+1.0);
}

void KisToolTransform::slotMoveDiscreteRightMore()
{
    setTranslateX(translateX()+10.0);
}

void KisToolTransform::slotUpdateToWarpType()
{
    setTransformMode(KisToolTransform::TransformToolMode::WarpTransformMode);
}

void KisToolTransform::slotUpdateToPerspectiveType()
{
    setTransformMode(KisToolTransform::TransformToolMode::PerspectiveTransformMode);
}

void KisToolTransform::slotUpdateToFreeTransformType()
{
    setTransformMode(KisToolTransform::TransformToolMode::FreeTransformMode);
}

void KisToolTransform::slotUpdateToLiquifyType()
{
    setTransformMode(KisToolTransform::TransformToolMode::LiquifyTransformMode);
}

void KisToolTransform::slotUpdateToMeshType()
{
    setTransformMode(KisToolTransform::TransformToolMode::MeshTransformMode);
}

void KisToolTransform::slotUpdateToCageType()
{
    setTransformMode(KisToolTransform::TransformToolMode::CageTransformMode);
}

void KisToolTransform::setShearY(double shear)
{
    m_optionsWidget->slotSetShearY(shear);
}

void KisToolTransform::setShearX(double shear)
{
    m_optionsWidget->slotSetShearX(shear);
}

void KisToolTransform::setScaleY(double scale)
{
    m_optionsWidget->slotSetScaleY(scale);
}

void KisToolTransform::setScaleX(double scale)
{
    m_optionsWidget->slotSetScaleX(scale);
}

void KisToolTransform::setTranslateY(double translation)
{
    m_optionsWidget->slotSetTranslateY(translation);
}

void KisToolTransform::setTranslateX(double translation)
{
    m_optionsWidget->slotSetTranslateX(translation);
}

QList<QAction *> KisToolTransformFactory::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions = KisToolPaintFactoryBase::createActionsImpl();

    actions << actionRegistry->makeQAction("movetool-move-up");
    actions << actionRegistry->makeQAction("movetool-move-down");
    actions << actionRegistry->makeQAction("movetool-move-left");
    actions << actionRegistry->makeQAction("movetool-move-right");
    actions << actionRegistry->makeQAction("movetool-move-up-more");
    actions << actionRegistry->makeQAction("movetool-move-down-more");
    actions << actionRegistry->makeQAction("movetool-move-left-more");
    actions << actionRegistry->makeQAction("movetool-move-right-more");

    return actions;
}
