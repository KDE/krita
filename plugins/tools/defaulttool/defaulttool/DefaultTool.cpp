/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2008 C. Boemann <cbo@boemann.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "DefaultTool.h"
#include "DefaultToolGeometryWidget.h"
#include "DefaultToolTabbedWidget.h"
#include "SelectionDecorator.h"
#include "ShapeMoveStrategy.h"
#include "ShapeRotateStrategy.h"
#include "ShapeShearStrategy.h"
#include "ShapeResizeStrategy.h"

#include <KoPointerEvent.h>
#include <KoToolSelection.h>
#include <KoToolManager.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoSelectedShapesProxy.h>
#include <KoShapeGroup.h>
#include <KoShapeLayer.h>
#include <KoPathShape.h>
#include <KoDrag.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeRubberSelectStrategy.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeTransformCommand.h>
#include <commands/KoShapeDeleteCommand.h>
#include <commands/KoShapeCreateCommand.h>
#include <commands/KoShapeGroupCommand.h>
#include <commands/KoShapeUngroupCommand.h>
#include <commands/KoShapeDistributeCommand.h>
#include <commands/KoKeepShapesSelectedCommand.h>
#include <KoSnapGuide.h>
#include <KoStrokeConfigWidget.h>
#include "kis_action_registry.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include <KoInteractionStrategyFactory.h>

#include "kis_document_aware_spin_box_unit_manager.h"

#include <KoIcon.h>

#include <QPainterPath>
#include <QPointer>
#include <QAction>
#include <QKeyEvent>
#include <KisSignalMapper.h>
#include <KoResourcePaths.h>

#include <KoCanvasController.h>
#include <kactioncollection.h>
#include <QMenu>

#include <math.h>
#include "kis_assert.h"
#include "kis_global.h"
#include "kis_debug.h"
#include "krita_utils.h"
#include "KisDocument.h"

#include <QVector2D>

#define HANDLE_DISTANCE 10
#define HANDLE_DISTANCE_SQ (HANDLE_DISTANCE * HANDLE_DISTANCE)

#define INNER_HANDLE_DISTANCE_SQ 16

namespace {
static const QString EditFillGradientFactoryId = "edit_fill_gradient";
static const QString EditStrokeGradientFactoryId = "edit_stroke_gradient";
static const QString EditFillMeshGradientFactoryId = "edit_fill_meshgradient";

enum TransformActionType {
    TransformRotate90CW,
    TransformRotate90CCW,
    TransformRotate180,
    TransformMirrorX,
    TransformMirrorY,
    TransformReset
};

enum BooleanOp {
    BooleanUnion,
    BooleanIntersection,
    BooleanSubtraction
};

}

class NopInteractionStrategy : public KoInteractionStrategy
{
public:
    explicit NopInteractionStrategy(KoToolBase *parent)
        : KoInteractionStrategy(parent)
    {
    }

    KUndo2Command *createCommand() override
    {
        return 0;
    }

    void handleMouseMove(const QPointF & /*mouseLocation*/, Qt::KeyboardModifiers /*modifiers*/) override {}
    void finishInteraction(Qt::KeyboardModifiers /*modifiers*/) override {}

    void paint(QPainter &painter, const KoViewConverter &converter) override {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class SelectionInteractionStrategy : public KoShapeRubberSelectStrategy
{
public:
    explicit SelectionInteractionStrategy(KoToolBase *parent, const QPointF &clicked, bool useSnapToGrid)
        : KoShapeRubberSelectStrategy(parent, clicked, useSnapToGrid)
    {
    }

    void paint(QPainter &painter, const KoViewConverter &converter) override {
        KoShapeRubberSelectStrategy::paint(painter, converter);
    }

    void cancelInteraction() override
    {
        tool()->canvas()->updateCanvas(selectedRectangle() | tool()->decorationsRect());
    }

    void finishInteraction(Qt::KeyboardModifiers modifiers = QFlags<Qt::KeyboardModifier>()) override
    {
        Q_UNUSED(modifiers);
        DefaultTool *defaultTool = dynamic_cast<DefaultTool*>(tool());
        KIS_SAFE_ASSERT_RECOVER_RETURN(defaultTool);

        KoSelection * selection = defaultTool->koSelection();

        const bool useContainedMode = currentMode() == CoveringSelection;

        QList<KoShape *> shapes =
                defaultTool->shapeManager()->
                        shapesAt(selectedRectangle(), true, useContainedMode);

        Q_FOREACH (KoShape * shape, shapes) {
                if (!shape->isSelectable()) continue;

                selection->select(shape);
            }

        tool()->canvas()->updateCanvas(selectedRectangle() | tool()->decorationsRect());
    }
};
#include <KoGradientBackground.h>
#include "KoShapeGradientHandles.h"
#include "ShapeGradientEditStrategy.h"

class DefaultTool::MoveGradientHandleInteractionFactory : public KoInteractionStrategyFactory
{
public:
    MoveGradientHandleInteractionFactory(KoFlake::FillVariant fillVariant,
                                         int priority, const QString &id, DefaultTool *_q)
        : KoInteractionStrategyFactory(priority, id),
          q(_q),
          m_fillVariant(fillVariant)
    {
    }

    KoInteractionStrategy* createStrategy(KoPointerEvent *ev) override
    {
        m_currentHandle = handleAt(ev->point);

        if (m_currentHandle.type != KoShapeGradientHandles::Handle::None) {
            KoShape *shape = onlyEditableShape();
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, 0);

            return new ShapeGradientEditStrategy(q, m_fillVariant, shape, m_currentHandle.type, ev->point);
        }

        return 0;
    }

    bool hoverEvent(KoPointerEvent *ev) override
    {
        m_currentHandle = handleAt(ev->point);
        return false;
    }

    bool paintOnHover(QPainter &painter, const KoViewConverter &converter) override
    {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        return false;
    }

    bool tryUseCustomCursor() override {
        if (m_currentHandle.type != KoShapeGradientHandles::Handle::None) {
            q->useCursor(Qt::OpenHandCursor);
            return true;
        }

        return false;
    }

private:

    KoShape* onlyEditableShape() const {
        KoSelection *selection = q->koSelection();
        QList<KoShape*> shapes = selection->selectedEditableShapes();

        KoShape *shape = 0;
        if (shapes.size() == 1) {
            shape = shapes.first();
        }

        return shape;
    }

    KoShapeGradientHandles::Handle handleAt(const QPointF &pos) {
        KoShapeGradientHandles::Handle result;

        KoShape *shape = onlyEditableShape();
        if (shape) {
            KoFlake::SelectionHandle globalHandle = q->handleAt(pos);
            const qreal distanceThresholdSq =
                globalHandle == KoFlake::NoHandle ?
                    HANDLE_DISTANCE_SQ : 0.25 * HANDLE_DISTANCE_SQ;

            const KoViewConverter *converter = q->canvas()->viewConverter();
            const QPointF viewPoint = converter->documentToView(pos);
            qreal minDistanceSq = std::numeric_limits<qreal>::max();

            KoShapeGradientHandles sh(m_fillVariant, shape);
            auto handless = sh.handles();
            Q_FOREACH (const KoShapeGradientHandles::Handle &handle, sh.handles()) {
                const QPointF handlePoint = converter->documentToView(handle.pos);
                const qreal distanceSq = kisSquareDistance(viewPoint, handlePoint);

                if (distanceSq < distanceThresholdSq && distanceSq < minDistanceSq) {
                    result = handle;
                    minDistanceSq = distanceSq;
                }
            }
        }

        return result;
    }

private:
    DefaultTool *q;
    KoFlake::FillVariant m_fillVariant;
    KoShapeGradientHandles::Handle m_currentHandle;
};

#include "KoShapeMeshGradientHandles.h"
#include "ShapeMeshGradientEditStrategy.h"

class DefaultTool::MoveMeshGradientHandleInteractionFactory: public KoInteractionStrategyFactory
{
public:
    MoveMeshGradientHandleInteractionFactory(KoFlake::FillVariant fillVariant,
                                             int priority,
                                             const QString& id,
                                             DefaultTool* _q)
        : KoInteractionStrategyFactory(priority, id)
        , m_fillVariant(fillVariant)
        , q(_q)
    {
    }

    KoInteractionStrategy* createStrategy(KoPointerEvent *ev) override
    {
        m_currentHandle = handleAt(ev->point);
        q->m_selectedMeshHandle = m_currentHandle;
        emit q->meshgradientHandleSelected(m_currentHandle);


        if (m_currentHandle.type != KoShapeMeshGradientHandles::Handle::None) {
            KoShape *shape = onlyEditableShape();
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, 0);

            return new ShapeMeshGradientEditStrategy(q, m_fillVariant, shape, m_currentHandle, ev->point);
        }

        return nullptr;
    }

    bool hoverEvent(KoPointerEvent *ev) override
    {
        // for custom cursor
        KoShapeMeshGradientHandles::Handle handle = handleAt(ev->point);

        // refresh
        if (handle.type != m_currentHandle.type && handle.type == KoShapeMeshGradientHandles::Handle::None) {
            q->repaintDecorations();
        }

        m_currentHandle = handle;
        q->m_hoveredMeshHandle = m_currentHandle;

        // highlight the decoration which is being hovered
        if (m_currentHandle.type != KoShapeMeshGradientHandles::Handle::None) {
            q->repaintDecorations();
        }
        return false;
    }

    bool paintOnHover(QPainter &painter, const KoViewConverter &converter) override
    {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        return false;
    }

    bool tryUseCustomCursor() override
    {
        if (m_currentHandle.type != KoShapeMeshGradientHandles::Handle::None) {
            q->useCursor(Qt::OpenHandCursor);
            return true;
        }

        return false;
    }


private:
    KoShape* onlyEditableShape() const {
        // FIXME: copy of KoShapeGradientHandles
        KoSelection *selection = q->koSelection();
        QList<KoShape*> shapes = selection->selectedEditableShapes();

        KoShape *shape = 0;
        if (shapes.size() == 1) {
            shape = shapes.first();
        }

        return shape;
    }

    KoShapeMeshGradientHandles::Handle handleAt(const QPointF &pos) const
    {
        // FIXME: copy of KoShapeGradientHandles. use a template?
        KoShapeMeshGradientHandles::Handle result;

        KoShape *shape = onlyEditableShape();
        if (shape) {
            KoFlake::SelectionHandle globalHandle = q->handleAt(pos);
            const qreal distanceThresholdSq =
                globalHandle == KoFlake::NoHandle ?
                    HANDLE_DISTANCE_SQ : 0.25 * HANDLE_DISTANCE_SQ;

            const KoViewConverter *converter = q->canvas()->viewConverter();
            const QPointF viewPoint = converter->documentToView(pos);
            qreal minDistanceSq = std::numeric_limits<qreal>::max();

            KoShapeMeshGradientHandles sh(m_fillVariant, shape);

            for (const auto& handle: sh.handles()) {
                const QPointF handlePoint = converter->documentToView(handle.pos);
                const qreal distanceSq = kisSquareDistance(viewPoint, handlePoint);

                if (distanceSq < distanceThresholdSq && distanceSq < minDistanceSq) {
                    result = handle;
                    minDistanceSq = distanceSq;
                }
            }
        }

        return result;
    }

private:
    KoFlake::FillVariant m_fillVariant;
    KoShapeMeshGradientHandles::Handle m_currentHandle;
    DefaultTool *q;
};

class SelectionHandler : public KoToolSelection
{
public:
    SelectionHandler(DefaultTool *parent)
        : KoToolSelection(parent)
        , m_selection(parent->koSelection())
    {
    }

    bool hasSelection() override
    {
        if (m_selection) {
            return m_selection->count();
        }
        return false;
    }

private:
    QPointer<KoSelection> m_selection;
};

DefaultTool::DefaultTool(KoCanvasBase *canvas, bool connectToSelectedShapesProxy)
    : KoInteractionTool(canvas)
    , m_lastHandle(KoFlake::NoHandle)
    , m_hotPosition(KoFlake::TopLeft)
    , m_mouseWasInsideHandles(false)
    , m_selectionHandler(new SelectionHandler(this))
    , m_tabbedOptionWidget(0)
{
    setupActions();

    QPixmap rotatePixmap, shearPixmap;
    rotatePixmap.load(":/cursor_rotate.png");
    Q_ASSERT(!rotatePixmap.isNull());
    shearPixmap.load(":/cursor_shear.png");
    Q_ASSERT(!shearPixmap.isNull());

    m_rotateCursors[0] = QCursor(rotatePixmap.transformed(QTransform().rotate(45)));
    m_rotateCursors[1] = QCursor(rotatePixmap.transformed(QTransform().rotate(90)));
    m_rotateCursors[2] = QCursor(rotatePixmap.transformed(QTransform().rotate(135)));
    m_rotateCursors[3] = QCursor(rotatePixmap.transformed(QTransform().rotate(180)));
    m_rotateCursors[4] = QCursor(rotatePixmap.transformed(QTransform().rotate(225)));
    m_rotateCursors[5] = QCursor(rotatePixmap.transformed(QTransform().rotate(270)));
    m_rotateCursors[6] = QCursor(rotatePixmap.transformed(QTransform().rotate(315)));
    m_rotateCursors[7] = QCursor(rotatePixmap);
    /*
        m_rotateCursors[0] = QCursor(Qt::RotateNCursor);
        m_rotateCursors[1] = QCursor(Qt::RotateNECursor);
        m_rotateCursors[2] = QCursor(Qt::RotateECursor);
        m_rotateCursors[3] = QCursor(Qt::RotateSECursor);
        m_rotateCursors[4] = QCursor(Qt::RotateSCursor);
        m_rotateCursors[5] = QCursor(Qt::RotateSWCursor);
        m_rotateCursors[6] = QCursor(Qt::RotateWCursor);
        m_rotateCursors[7] = QCursor(Qt::RotateNWCursor);
    */
    m_shearCursors[0] = QCursor(shearPixmap);
    m_shearCursors[1] = QCursor(shearPixmap.transformed(QTransform().rotate(45)));
    m_shearCursors[2] = QCursor(shearPixmap.transformed(QTransform().rotate(90)));
    m_shearCursors[3] = QCursor(shearPixmap.transformed(QTransform().rotate(135)));
    m_shearCursors[4] = QCursor(shearPixmap.transformed(QTransform().rotate(180)));
    m_shearCursors[5] = QCursor(shearPixmap.transformed(QTransform().rotate(225)));
    m_shearCursors[6] = QCursor(shearPixmap.transformed(QTransform().rotate(270)));
    m_shearCursors[7] = QCursor(shearPixmap.transformed(QTransform().rotate(315)));
    m_sizeCursors[0] = Qt::SizeVerCursor;
    m_sizeCursors[1] = Qt::SizeBDiagCursor;
    m_sizeCursors[2] = Qt::SizeHorCursor;
    m_sizeCursors[3] = Qt::SizeFDiagCursor;
    m_sizeCursors[4] = Qt::SizeVerCursor;
    m_sizeCursors[5] = Qt::SizeBDiagCursor;
    m_sizeCursors[6] = Qt::SizeHorCursor;
    m_sizeCursors[7] = Qt::SizeFDiagCursor;

    if (connectToSelectedShapesProxy) {
        connect(canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(updateActions()));

        connect(canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(repaintDecorations()));
        connect(canvas->selectedShapesProxy(), SIGNAL(selectionContentChanged()), this, SLOT(repaintDecorations()));
    }
}

DefaultTool::~DefaultTool()
{
}

void DefaultTool::slotActivateEditFillGradient(bool value)
{
    if (value) {
        addInteractionFactory(
            new MoveGradientHandleInteractionFactory(KoFlake::Fill,
                                                     1, EditFillGradientFactoryId, this));
    } else {
        removeInteractionFactory(EditFillGradientFactoryId);
    }
    repaintDecorations();
}

void DefaultTool::slotActivateEditStrokeGradient(bool value)
{
    if (value) {
        addInteractionFactory(
            new MoveGradientHandleInteractionFactory(KoFlake::StrokeFill,
                                                     0, EditStrokeGradientFactoryId, this));
    } else {
        removeInteractionFactory(EditStrokeGradientFactoryId);
    }
    repaintDecorations();
}

void DefaultTool::slotActivateEditFillMeshGradient(bool value)
{
    if (value) {
        connect(this, SIGNAL(meshgradientHandleSelected(KoShapeMeshGradientHandles::Handle)),
                m_tabbedOptionWidget, SLOT(slotMeshGradientHandleSelected(KoShapeMeshGradientHandles::Handle)));
        addInteractionFactory(
            new MoveMeshGradientHandleInteractionFactory(KoFlake::Fill, 1,
                                                         EditFillMeshGradientFactoryId, this));
    } else {
        disconnect(this, SIGNAL(meshgradientHandleSelected(KoShapeMeshGradientHandles::Handle)),
                   m_tabbedOptionWidget, SLOT(slotMeshGradientHandleSelected(KoShapeMeshGradientHandles::Handle)));
        removeInteractionFactory(EditFillMeshGradientFactoryId);
    }
}

void DefaultTool::slotResetMeshGradientState()
{
    m_selectedMeshHandle = KoShapeMeshGradientHandles::Handle();
}

bool DefaultTool::wantsAutoScroll() const
{
    return true;
}

void DefaultTool::addMappedAction(KisSignalMapper *mapper, const QString &actionId, int commandType)
{
    QAction *a =action(actionId);
    connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(a, commandType);
}

void DefaultTool::setupActions()
{
    m_alignSignalsMapper = new KisSignalMapper(this);

    addMappedAction(m_alignSignalsMapper, "object_align_horizontal_left", KoShapeAlignCommand::HorizontalLeftAlignment);
    addMappedAction(m_alignSignalsMapper, "object_align_horizontal_center", KoShapeAlignCommand::HorizontalCenterAlignment);
    addMappedAction(m_alignSignalsMapper, "object_align_horizontal_right", KoShapeAlignCommand::HorizontalRightAlignment);
    addMappedAction(m_alignSignalsMapper, "object_align_vertical_top", KoShapeAlignCommand::VerticalTopAlignment);
    addMappedAction(m_alignSignalsMapper, "object_align_vertical_center", KoShapeAlignCommand::VerticalCenterAlignment);
    addMappedAction(m_alignSignalsMapper, "object_align_vertical_bottom", KoShapeAlignCommand::VerticalBottomAlignment);

    m_distributeSignalsMapper = new KisSignalMapper(this);

    addMappedAction(m_distributeSignalsMapper, "object_distribute_horizontal_left", KoShapeDistributeCommand::HorizontalLeftDistribution);
    addMappedAction(m_distributeSignalsMapper, "object_distribute_horizontal_center", KoShapeDistributeCommand::HorizontalCenterDistribution);
    addMappedAction(m_distributeSignalsMapper, "object_distribute_horizontal_right", KoShapeDistributeCommand::HorizontalRightDistribution);
    addMappedAction(m_distributeSignalsMapper, "object_distribute_horizontal_gaps", KoShapeDistributeCommand::HorizontalGapsDistribution);

    addMappedAction(m_distributeSignalsMapper, "object_distribute_vertical_top", KoShapeDistributeCommand::VerticalTopDistribution);
    addMappedAction(m_distributeSignalsMapper, "object_distribute_vertical_center", KoShapeDistributeCommand::VerticalCenterDistribution);
    addMappedAction(m_distributeSignalsMapper, "object_distribute_vertical_bottom", KoShapeDistributeCommand::VerticalBottomDistribution);
    addMappedAction(m_distributeSignalsMapper, "object_distribute_vertical_gaps", KoShapeDistributeCommand::VerticalGapsDistribution);

    m_transformSignalsMapper = new KisSignalMapper(this);

    addMappedAction(m_transformSignalsMapper, "object_transform_rotate_90_cw", TransformRotate90CW);
    addMappedAction(m_transformSignalsMapper, "object_transform_rotate_90_ccw", TransformRotate90CCW);
    addMappedAction(m_transformSignalsMapper, "object_transform_rotate_180", TransformRotate180);
    addMappedAction(m_transformSignalsMapper, "object_transform_mirror_horizontally", TransformMirrorX);
    addMappedAction(m_transformSignalsMapper, "object_transform_mirror_vertically", TransformMirrorY);
    addMappedAction(m_transformSignalsMapper, "object_transform_reset", TransformReset);

    m_booleanSignalsMapper = new KisSignalMapper(this);

    addMappedAction(m_booleanSignalsMapper, "object_unite", BooleanUnion);
    addMappedAction(m_booleanSignalsMapper, "object_intersect", BooleanIntersection);
    addMappedAction(m_booleanSignalsMapper, "object_subtract", BooleanSubtraction);

    m_contextMenu.reset(new QMenu());
}

qreal DefaultTool::rotationOfHandle(KoFlake::SelectionHandle handle, bool useEdgeRotation)
{
    QPointF selectionCenter = koSelection()->absolutePosition();
    QPointF direction;

    switch (handle) {
    case KoFlake::TopMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::TopRight)
                        - koSelection()->absolutePosition(KoFlake::TopLeft);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::TopLeft);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::TopRight) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::TopRightHandle:
        direction = (QVector2D(koSelection()->absolutePosition(KoFlake::TopRight) - koSelection()->absolutePosition(KoFlake::TopLeft)).normalized() + QVector2D(koSelection()->absolutePosition(KoFlake::TopRight) - koSelection()->absolutePosition(KoFlake::BottomRight)).normalized()).toPointF();
        break;
    case KoFlake::RightMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::BottomRight)
                        - koSelection()->absolutePosition(KoFlake::TopRight);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::TopRight);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::BottomRight) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::BottomRightHandle:
        direction = (QVector2D(koSelection()->absolutePosition(KoFlake::BottomRight) - koSelection()->absolutePosition(KoFlake::BottomLeft)).normalized() + QVector2D(koSelection()->absolutePosition(KoFlake::BottomRight) - koSelection()->absolutePosition(KoFlake::TopRight)).normalized()).toPointF();
        break;
    case KoFlake::BottomMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::BottomLeft)
                        - koSelection()->absolutePosition(KoFlake::BottomRight);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::BottomLeft);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::BottomRight) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::BottomLeftHandle:
        direction = koSelection()->absolutePosition(KoFlake::BottomLeft) - selectionCenter;
        direction = (QVector2D(koSelection()->absolutePosition(KoFlake::BottomLeft) - koSelection()->absolutePosition(KoFlake::BottomRight)).normalized() + QVector2D(koSelection()->absolutePosition(KoFlake::BottomLeft) - koSelection()->absolutePosition(KoFlake::TopLeft)).normalized()).toPointF();
        break;
    case KoFlake::LeftMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::TopLeft)
                        - koSelection()->absolutePosition(KoFlake::BottomLeft);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::TopLeft);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::BottomLeft) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::TopLeftHandle:
        direction = koSelection()->absolutePosition(KoFlake::TopLeft) - selectionCenter;
        direction = (QVector2D(koSelection()->absolutePosition(KoFlake::TopLeft) - koSelection()->absolutePosition(KoFlake::TopRight)).normalized() + QVector2D(koSelection()->absolutePosition(KoFlake::TopLeft) - koSelection()->absolutePosition(KoFlake::BottomLeft)).normalized()).toPointF();
        break;
    case KoFlake::NoHandle:
        return 0.0;
        break;
    }

    qreal rotation = atan2(direction.y(), direction.x()) * 180.0 / M_PI;

    switch (handle) {
    case KoFlake::TopMiddleHandle:
        if (useEdgeRotation) {
            rotation -= 0.0;
        } else {
            rotation -= 270.0;
        }
        break;
    case KoFlake::TopRightHandle:
        rotation -= 315.0;
        break;
    case KoFlake::RightMiddleHandle:
        if (useEdgeRotation) {
            rotation -= 90.0;
        } else {
            rotation -= 0.0;
        }
        break;
    case KoFlake::BottomRightHandle:
        rotation -= 45.0;
        break;
    case KoFlake::BottomMiddleHandle:
        if (useEdgeRotation) {
            rotation -= 180.0;
        } else {
            rotation -= 90.0;
        }
        break;
    case KoFlake::BottomLeftHandle:
        rotation -= 135.0;
        break;
    case KoFlake::LeftMiddleHandle:
        if (useEdgeRotation) {
            rotation -= 270.0;
        } else {
            rotation -= 180.0;
        }
        break;
    case KoFlake::TopLeftHandle:
        rotation -= 225.0;
        break;
    default:
        ;
    }

    if (rotation < 0.0) {
        rotation += 360.0;
    }

    return rotation;
}

void DefaultTool::updateCursor()
{
    if (tryUseCustomCursor()) return;

    QCursor cursor = Qt::ArrowCursor;

    QString statusText;

    KoSelection *selection = koSelection();
    if (selection && selection->count() > 0) { // has a selection
        bool editable = !selection->selectedEditableShapes().isEmpty();

        if (!m_mouseWasInsideHandles) {
            m_angle = rotationOfHandle(m_lastHandle, true);
            int rotOctant = 8 + int(8.5 + m_angle / 45);

            bool rotateHandle = false;
            bool shearHandle = false;
            switch (m_lastHandle) {
            case KoFlake::TopMiddleHandle:
                cursor = m_shearCursors[(0 + rotOctant) % 8];
                shearHandle = true;
                break;
            case KoFlake::TopRightHandle:
                cursor = m_rotateCursors[(1 + rotOctant) % 8];
                rotateHandle = true;
                break;
            case KoFlake::RightMiddleHandle:
                cursor = m_shearCursors[(2 + rotOctant) % 8];
                shearHandle = true;
                break;
            case KoFlake::BottomRightHandle:
                cursor = m_rotateCursors[(3 + rotOctant) % 8];
                rotateHandle = true;
                break;
            case KoFlake::BottomMiddleHandle:
                cursor = m_shearCursors[(4 + rotOctant) % 8];
                shearHandle = true;
                break;
            case KoFlake::BottomLeftHandle:
                cursor = m_rotateCursors[(5 + rotOctant) % 8];
                rotateHandle = true;
                break;
            case KoFlake::LeftMiddleHandle:
                cursor = m_shearCursors[(6 + rotOctant) % 8];
                shearHandle = true;
                break;
            case KoFlake::TopLeftHandle:
                cursor = m_rotateCursors[(7 + rotOctant) % 8];
                rotateHandle = true;
                break;
            case KoFlake::NoHandle:
                cursor = Qt::ArrowCursor;
                break;
            }
            if (rotateHandle) {
                statusText = i18n("Left click rotates around center, right click around highlighted position.");
            }
            if (shearHandle) {
                statusText = i18n("Click and drag to shear selection.");
            }


        } else {
            statusText = i18n("Click and drag to resize selection.");
            m_angle = rotationOfHandle(m_lastHandle, false);
            int rotOctant = 8 + int(8.5 + m_angle / 45);
            bool cornerHandle = false;
            switch (m_lastHandle) {
            case KoFlake::TopMiddleHandle:
                cursor = m_sizeCursors[(0 + rotOctant) % 8];
                break;
            case KoFlake::TopRightHandle:
                cursor = m_sizeCursors[(1 + rotOctant) % 8];
                cornerHandle = true;
                break;
            case KoFlake::RightMiddleHandle:
                cursor = m_sizeCursors[(2 + rotOctant) % 8];
                break;
            case KoFlake::BottomRightHandle:
                cursor = m_sizeCursors[(3 + rotOctant) % 8];
                cornerHandle = true;
                break;
            case KoFlake::BottomMiddleHandle:
                cursor = m_sizeCursors[(4 + rotOctant) % 8];
                break;
            case KoFlake::BottomLeftHandle:
                cursor = m_sizeCursors[(5 + rotOctant) % 8];
                cornerHandle = true;
                break;
            case KoFlake::LeftMiddleHandle:
                cursor = m_sizeCursors[(6 + rotOctant) % 8];
                break;
            case KoFlake::TopLeftHandle:
                cursor = m_sizeCursors[(7 + rotOctant) % 8];
                cornerHandle = true;
                break;
            case KoFlake::NoHandle:
                cursor = Qt::SizeAllCursor;
                statusText = i18n("Click and drag to move selection.");
                break;
            }
            if (cornerHandle) {
                statusText = i18n("Click and drag to resize selection. Middle click to set highlighted position.");
            }
        }
        if (!editable) {
            cursor = Qt::ArrowCursor;
        }
    } else {
        // there used to be guides... :'''(
    }
    useCursor(cursor);
    if (currentStrategy() == 0) {
        emit statusTextChanged(statusText);
    }
}

void DefaultTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    KoSelection *selection = koSelection();
    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(canvas());
    if (selection) {
        m_decorator.reset(new SelectionDecorator(canvas()->resourceManager()));

        {
            /**
             * Selection masks don't render the outline of the shapes, so we should
             * do that explicitly when rendering them via selection
             */

            KisNodeSP node = kisCanvas->viewManager()->nodeManager()->activeNode();
            const bool isSelectionMask = node && node->inherits("KisSelectionMask");
            m_decorator->setForceShapeOutlines(isSelectionMask);
        }

        m_decorator->setSelection(selection);
        m_decorator->setHandleRadius(handleRadius());
        m_decorator->setShowFillGradientHandles(hasInteractioFactory(EditFillGradientFactoryId));
        m_decorator->setShowStrokeFillGradientHandles(hasInteractioFactory(EditStrokeGradientFactoryId));
        m_decorator->setShowFillMeshGradientHandles(hasInteractioFactory(EditFillMeshGradientFactoryId));
        m_decorator->setCurrentMeshGradientHandles(m_selectedMeshHandle, m_hoveredMeshHandle);
        m_decorator->paint(painter, converter);
    }

    KoInteractionTool::paint(painter, converter);

    painter.save();
    painter.setTransform(converter.documentToView(), true);
    canvas()->snapGuide()->paint(painter, converter);
    painter.restore();
}

bool DefaultTool::isValidForCurrentLayer() const
{
    // if the currently active node has a shape manager, then it is
    // probably our client :)

    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(canvas());
    return bool(kisCanvas->localShapeManager());
}

KoShapeManager *DefaultTool::shapeManager() const {
    return canvas()->shapeManager();
}

void DefaultTool::mousePressEvent(KoPointerEvent *event)
{
    // this tool only works on a vector layer right now, so give a warning if another layer type is trying to use it
    if (!isValidForCurrentLayer()) {
        KisCanvas2 *kiscanvas = static_cast<KisCanvas2 *>(canvas());
        kiscanvas->viewManager()->showFloatingMessage(
                i18n("This tool only works on vector layers. You probably want the move tool."),
                QIcon(), 2000, KisFloatingMessage::Medium, Qt::AlignCenter);
        return;
    }

    KoInteractionTool::mousePressEvent(event);
    updateCursor();
}

void DefaultTool::mouseMoveEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseMoveEvent(event);
    if (currentStrategy() == 0 && koSelection() && koSelection()->count() > 0) {
        QRectF bound = handlesSize();

        if (bound.contains(event->point)) {
            bool inside;
            KoFlake::SelectionHandle newDirection = handleAt(event->point, &inside);

            if (inside != m_mouseWasInsideHandles || m_lastHandle != newDirection) {
                m_lastHandle = newDirection;
                m_mouseWasInsideHandles = inside;
            }
        } else {
            m_lastHandle = KoFlake::NoHandle;
            m_mouseWasInsideHandles = false;

            // there used to be guides... :'''(
        }
    } else {
        // there used to be guides... :'''(
    }


    updateCursor();
}

QRectF DefaultTool::handlesSize()
{
    KoSelection *selection = koSelection();
    if (!selection || !selection->count()) return QRectF();

    recalcSelectionBox(selection);

    QRectF bound = m_selectionOutline.boundingRect();

    // expansion Border
    if (!canvas() || !canvas()->viewConverter()) {
        return bound;
    }

    QPointF border = canvas()->viewConverter()->viewToDocument(QPointF(HANDLE_DISTANCE, HANDLE_DISTANCE));
    bound.adjust(-border.x(), -border.y(), border.x(), border.y());
    return bound;
}

void DefaultTool::mouseReleaseEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseReleaseEvent(event);
    updateCursor();
}

void DefaultTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    KoSelection *selection = koSelection();

    KoShape *shape = shapeManager()->shapeAt(event->point, KoFlake::ShapeOnTop);
    if (shape && selection && !selection->isSelected(shape)) {

        if (!(event->modifiers() & Qt::ShiftModifier)) {
            selection->deselectAll();
        }

        selection->select(shape);
    }

    explicitUserStrokeEndRequest();
}

bool DefaultTool::moveSelection(int direction, Qt::KeyboardModifiers modifiers)
{
    bool result = false;

    qreal x = 0.0, y = 0.0;
    if (direction == Qt::Key_Left) {
        x = -5;
    } else if (direction == Qt::Key_Right) {
        x = 5;
    } else if (direction == Qt::Key_Up) {
        y = -5;
    } else if (direction == Qt::Key_Down) {
        y = 5;
    }

    if (x != 0.0 || y != 0.0) { // actually move

        if ((modifiers & Qt::ShiftModifier) != 0) {
            x *= 10;
            y *= 10;
        } else if ((modifiers & Qt::AltModifier) != 0) { // more precise
            x /= 5;
            y /= 5;
        }

        QList<KoShape *> shapes = koSelection()->selectedEditableShapes();

        if (!shapes.isEmpty()) {
            canvas()->addCommand(new KoShapeMoveCommand(shapes, QPointF(x, y)));
            result = true;
        }
    }

    return result;
}

void DefaultTool::keyPressEvent(QKeyEvent *event)
{
    KoInteractionTool::keyPressEvent(event);
    if (currentStrategy() == 0) {
        switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (moveSelection(event->key(), event->modifiers())) {
                event->accept();
            }
            break;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
            canvas()->resourceManager()->setResource(HotPosition, event->key() - Qt::Key_1);
            event->accept();
            break;
        default:
            return;
        }
    }
}

QRectF DefaultTool::decorationsRect() const
{
    QRectF dirtyRect;

    if (koSelection() && koSelection()->count() > 0) {
        /// TODO: avoid cons_cast by implementing proper
        ///       caching strategy inrecalcSelectionBox() and
        ///       handlesSize()
        dirtyRect = const_cast<DefaultTool*>(this)->handlesSize();
    }

    if (canvas()->snapGuide()->isSnapping()) {
        dirtyRect |= canvas()->snapGuide()->boundingRect();
    }

    return dirtyRect;
}

void DefaultTool::copy() const
{
    // all the selected shapes, not only editable!
    QList<KoShape *> shapes = koSelection()->selectedShapes();

    if (!shapes.isEmpty()) {
        KoDrag drag;
        drag.setSvg(shapes);
        drag.addToClipboard();
    }
}

void DefaultTool::deleteSelection()
{
    QList<KoShape *> shapes;
    foreach (KoShape *s, koSelection()->selectedShapes()) {
        if (s->isGeometryProtected()) {
            continue;
        }
        shapes << s;
    }
    if (!shapes.empty()) {
        canvas()->addCommand(canvas()->shapeController()->removeShapes(shapes));
    }
}

bool DefaultTool::paste()
{
    // we no longer have to do anything as tool Proxy will do it for us
    return false;
}

KoSelection *DefaultTool::koSelection() const
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->selectedShapesProxy());
    return canvas()->selectedShapesProxy()->selection();
}

KoFlake::SelectionHandle DefaultTool::handleAt(const QPointF &point, bool *innerHandleMeaning)
{
    // check for handles in this order; meaning that when handles overlap the one on top is chosen
    static const KoFlake::SelectionHandle handleOrder[] = {
        KoFlake::BottomRightHandle,
        KoFlake::TopLeftHandle,
        KoFlake::BottomLeftHandle,
        KoFlake::TopRightHandle,
        KoFlake::BottomMiddleHandle,
        KoFlake::RightMiddleHandle,
        KoFlake::LeftMiddleHandle,
        KoFlake::TopMiddleHandle,
        KoFlake::NoHandle
    };

    const KoViewConverter *converter = canvas()->viewConverter();
    KoSelection *selection = koSelection();

    if (!selection || !selection->count() || !converter) {
        return KoFlake::NoHandle;
    }

    recalcSelectionBox(selection);

    if (innerHandleMeaning) {
        QPainterPath path;
        path.addPolygon(m_selectionOutline);
        *innerHandleMeaning = path.contains(point) || path.intersects(handlePaintRect(point));
    }

    const QPointF viewPoint = converter->documentToView(point);

    for (int i = 0; i < KoFlake::NoHandle; ++i) {
        KoFlake::SelectionHandle handle = handleOrder[i];

        const QPointF handlePoint = converter->documentToView(m_selectionBox[handle]);
        const qreal distanceSq = kisSquareDistance(viewPoint, handlePoint);

        // if just inside the outline
        if (distanceSq < HANDLE_DISTANCE_SQ) {

            if (innerHandleMeaning) {
                if (distanceSq < INNER_HANDLE_DISTANCE_SQ) {
                    *innerHandleMeaning = true;
                }
            }

            return handle;
        }
    }
    return KoFlake::NoHandle;
}

void DefaultTool::recalcSelectionBox(KoSelection *selection)
{
    KIS_ASSERT_RECOVER_RETURN(selection->count());

    QTransform matrix = selection->absoluteTransformation();
    m_selectionOutline = matrix.map(QPolygonF(selection->outlineRect()));
    m_angle = 0.0;

    QPolygonF outline = m_selectionOutline; //shorter name in the following :)
    m_selectionBox[KoFlake::TopMiddleHandle] = (outline.value(0) + outline.value(1)) / 2;
    m_selectionBox[KoFlake::TopRightHandle] = outline.value(1);
    m_selectionBox[KoFlake::RightMiddleHandle] = (outline.value(1) + outline.value(2)) / 2;
    m_selectionBox[KoFlake::BottomRightHandle] = outline.value(2);
    m_selectionBox[KoFlake::BottomMiddleHandle] = (outline.value(2) + outline.value(3)) / 2;
    m_selectionBox[KoFlake::BottomLeftHandle] = outline.value(3);
    m_selectionBox[KoFlake::LeftMiddleHandle] = (outline.value(3) + outline.value(0)) / 2;
    m_selectionBox[KoFlake::TopLeftHandle] = outline.value(0);
    if (selection->count() == 1) {
#if 0        // TODO detect mirroring
        KoShape *s = koSelection()->firstSelectedShape();

        if (s->scaleX() < 0) { // vertically mirrored: swap left / right
            std::swap(m_selectionBox[KoFlake::TopLeftHandle], m_selectionBox[KoFlake::TopRightHandle]);
            std::swap(m_selectionBox[KoFlake::LeftMiddleHandle], m_selectionBox[KoFlake::RightMiddleHandle]);
            std::swap(m_selectionBox[KoFlake::BottomLeftHandle], m_selectionBox[KoFlake::BottomRightHandle]);
        }
        if (s->scaleY() < 0) { // vertically mirrored: swap top / bottom
            std::swap(m_selectionBox[KoFlake::TopLeftHandle], m_selectionBox[KoFlake::BottomLeftHandle]);
            std::swap(m_selectionBox[KoFlake::TopMiddleHandle], m_selectionBox[KoFlake::BottomMiddleHandle]);
            std::swap(m_selectionBox[KoFlake::TopRightHandle], m_selectionBox[KoFlake::BottomRightHandle]);
        }
#endif
    }
}

void DefaultTool::activate(const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(shapes);

    QAction *actionBringToFront = action("object_order_front");
    connect(actionBringToFront, SIGNAL(triggered()), this, SLOT(selectionBringToFront()), Qt::UniqueConnection);

    QAction *actionRaise = action("object_order_raise");
    connect(actionRaise, SIGNAL(triggered()), this, SLOT(selectionMoveUp()), Qt::UniqueConnection);

    QAction *actionLower = action("object_order_lower");
    connect(actionLower, SIGNAL(triggered()), this, SLOT(selectionMoveDown()));

    QAction *actionSendToBack = action("object_order_back");
    connect(actionSendToBack, SIGNAL(triggered()), this, SLOT(selectionSendToBack()), Qt::UniqueConnection);

    QAction *actionGroupBottom = action("object_group");
    connect(actionGroupBottom, SIGNAL(triggered()), this, SLOT(selectionGroup()), Qt::UniqueConnection);

    QAction *actionUngroupBottom = action("object_ungroup");
    connect(actionUngroupBottom, SIGNAL(triggered()), this, SLOT(selectionUngroup()), Qt::UniqueConnection);

    QAction *actionSplit = action("object_split");
    connect(actionSplit, SIGNAL(triggered()), this, SLOT(selectionSplitShapes()), Qt::UniqueConnection);

    connect(m_alignSignalsMapper, SIGNAL(mapped(int)), SLOT(selectionAlign(int)));
    connect(m_distributeSignalsMapper, SIGNAL(mapped(int)), SLOT(selectionDistribute(int)));
    connect(m_transformSignalsMapper, SIGNAL(mapped(int)), SLOT(selectionTransform(int)));
    connect(m_booleanSignalsMapper, SIGNAL(mapped(int)), SLOT(selectionBooleanOp(int)));

    m_mouseWasInsideHandles = false;
    m_lastHandle = KoFlake::NoHandle;
    useCursor(Qt::ArrowCursor);
    repaintDecorations();
    updateActions();

    if (m_tabbedOptionWidget) {
        m_tabbedOptionWidget->activate();
    }
}

void DefaultTool::deactivate()
{
    KoToolBase::deactivate();

    QAction *actionBringToFront = action("object_order_front");
    disconnect(actionBringToFront, 0, this, 0);

    QAction *actionRaise = action("object_order_raise");
    disconnect(actionRaise, 0, this, 0);

    QAction *actionLower = action("object_order_lower");
    disconnect(actionLower, 0, this, 0);

    QAction *actionSendToBack = action("object_order_back");
    disconnect(actionSendToBack, 0, this, 0);

    QAction *actionGroupBottom = action("object_group");
    disconnect(actionGroupBottom, 0, this, 0);

    QAction *actionUngroupBottom = action("object_ungroup");
    disconnect(actionUngroupBottom, 0, this, 0);

    QAction *actionSplit = action("object_split");
    disconnect(actionSplit, 0, this, 0);

    disconnect(m_alignSignalsMapper, 0, this, 0);
    disconnect(m_distributeSignalsMapper, 0, this, 0);
    disconnect(m_transformSignalsMapper, 0, this, 0);
    disconnect(m_booleanSignalsMapper, 0, this, 0);


    if (m_tabbedOptionWidget) {
        m_tabbedOptionWidget->deactivate();
    }
}

void DefaultTool::selectionGroup()
{
    KoSelection *selection = koSelection();
    if (!selection) return;

    QList<KoShape *> selectedShapes = selection->selectedEditableShapes();
    std::sort(selectedShapes.begin(), selectedShapes.end(), KoShape::compareShapeZIndex);
    if (selectedShapes.isEmpty()) return;

    const int groupZIndex = selectedShapes.last()->zIndex();

    KoShapeGroup *group = new KoShapeGroup();
    group->setZIndex(groupZIndex);
    // TODO what if only one shape is left?
    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Group shapes"));
    new KoKeepShapesSelectedCommand(selectedShapes, {}, canvas()->selectedShapesProxy(), false, cmd);
    canvas()->shapeController()->addShapeDirect(group, 0, cmd);
    new KoShapeGroupCommand(group, selectedShapes, true, cmd);
    new KoKeepShapesSelectedCommand({}, {group}, canvas()->selectedShapesProxy(), true, cmd);
    canvas()->addCommand(cmd);

    // update selection so we can ungroup immediately again
    selection->deselectAll();
    selection->select(group);
}

void DefaultTool::selectionUngroup()
{
    KoSelection *selection = koSelection();
    if (!selection) return;

    QList<KoShape *> selectedShapes = selection->selectedEditableShapes();
    std::sort(selectedShapes.begin(), selectedShapes.end(), KoShape::compareShapeZIndex);

    KUndo2Command *cmd = 0;
    QList<KoShape*> newShapes;

    // add a ungroup command for each found shape container to the macro command
    Q_FOREACH (KoShape *shape, selectedShapes) {
        KoShapeGroup *group = dynamic_cast<KoShapeGroup *>(shape);
        if (group) {
            if (!cmd) {
                cmd = new KUndo2Command(kundo2_i18n("Ungroup shapes"));
                new KoKeepShapesSelectedCommand(selectedShapes, {}, canvas()->selectedShapesProxy(), false, cmd);
            }
            newShapes << group->shapes();
            new KoShapeUngroupCommand(group, group->shapes(),
                                      group->parent() ? QList<KoShape *>() : shapeManager()->topLevelShapes(),
                                      cmd);
            canvas()->shapeController()->removeShape(group, cmd);
        }
    }
    if (cmd) {
        new KoKeepShapesSelectedCommand({}, newShapes, canvas()->selectedShapesProxy(), true, cmd);
        canvas()->addCommand(cmd);
    }
}

void DefaultTool::selectionTransform(int transformAction)
{
    KoSelection *selection = koSelection();
    if (!selection) return;

    QList<KoShape *> editableShapes = selection->selectedEditableShapes();
    if (editableShapes.isEmpty()) {
        return;
    }

    QTransform applyTransform;
    bool shouldReset = false;
    KUndo2MagicString actionName = kundo2_noi18n("BUG: No transform action");


    switch (TransformActionType(transformAction)) {
    case TransformRotate90CW:
        applyTransform.rotate(90.0);
        actionName = kundo2_i18n("Rotate Object 90° CW");
        break;
    case TransformRotate90CCW:
        applyTransform.rotate(-90.0);
        actionName = kundo2_i18n("Rotate Object 90° CCW");
        break;
    case TransformRotate180:
        applyTransform.rotate(180.0);
        actionName = kundo2_i18n("Rotate Object 180°");
        break;
    case TransformMirrorX:
        applyTransform.scale(-1.0, 1.0);
        actionName = kundo2_i18n("Mirror Object Horizontally");
        break;
    case TransformMirrorY:
        applyTransform.scale(1.0, -1.0);
        actionName = kundo2_i18n("Mirror Object Vertically");
        break;
    case TransformReset:
        shouldReset = true;
        actionName = kundo2_i18n("Reset Object Transformations");
        break;
    }

    if (!shouldReset && applyTransform.isIdentity()) return;

    QList<QTransform> oldTransforms;
    QList<QTransform> newTransforms;

    const QRectF outlineRect = KoShape::absoluteOutlineRect(editableShapes);
    const QPointF centerPoint = outlineRect.center();
    const QTransform centerTrans = QTransform::fromTranslate(centerPoint.x(), centerPoint.y());
    const QTransform centerTransInv = QTransform::fromTranslate(-centerPoint.x(), -centerPoint.y());

    // we also add selection to the list of transformed shapes, so that its outline is updated correctly
    QList<KoShape*> transformedShapes = editableShapes;
    transformedShapes << selection;

    Q_FOREACH (KoShape *shape, transformedShapes) {
        oldTransforms.append(shape->transformation());

        QTransform t;

        if (!shouldReset) {
            const QTransform world = shape->absoluteTransformation();
            t =  world * centerTransInv * applyTransform * centerTrans * world.inverted() * shape->transformation();
        } else {
            const QPointF center = shape->outlineRect().center();
            const QPointF offset = shape->transformation().map(center) - center;
            t = QTransform::fromTranslate(offset.x(), offset.y());
        }

        newTransforms.append(t);
    }

    KoShapeTransformCommand *cmd = new KoShapeTransformCommand(transformedShapes, oldTransforms, newTransforms);
    cmd->setText(actionName);
    canvas()->addCommand(cmd);
}

void DefaultTool::selectionBooleanOp(int booleanOp)
{
    KoSelection *selection = koSelection();
    if (!selection) return;

    QList<KoShape *> editableShapes = selection->selectedEditableShapes();
    if (editableShapes.isEmpty()) {
        return;
    }

    QVector<QPainterPath> srcOutlines;
    QPainterPath dstOutline;
    KUndo2MagicString actionName = kundo2_noi18n("BUG: boolean action name");

    // TODO: implement a reference shape selection dialog!
    const int referenceShapeIndex = 0;
    KoShape *referenceShape = editableShapes[referenceShapeIndex];

    KisCanvas2 *kisCanvas = static_cast<KisCanvas2 *>(canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
    const QTransform booleanWorkaroundTransform =
        KritaUtils::pathShapeBooleanSpaceWorkaround(kisCanvas->image());

    Q_FOREACH (KoShape *shape, editableShapes) {
        srcOutlines <<
            booleanWorkaroundTransform.map(
            shape->absoluteTransformation().map(
                shape->outline()));
    }

    if (booleanOp == BooleanUnion) {
        Q_FOREACH (const QPainterPath &path, srcOutlines) {
            dstOutline |= path;
        }
        actionName = kundo2_i18n("Unite Shapes");
    } else if (booleanOp == BooleanIntersection) {
        for (int i = 0; i < srcOutlines.size(); i++) {
            if (i == 0) {
                dstOutline = srcOutlines[i];
            } else {
                dstOutline &= srcOutlines[i];
            }
        }

        // there is a bug in Qt, sometimes it leaves the resulting
        // outline open, so just close it explicitly.
        dstOutline.closeSubpath();

        actionName = kundo2_i18n("Intersect Shapes");

    } else if (booleanOp == BooleanSubtraction) {
        for (int i = 0; i < srcOutlines.size(); i++) {
            dstOutline = srcOutlines[referenceShapeIndex];
            if (i != referenceShapeIndex) {
                dstOutline -= srcOutlines[i];
            }
        }

        actionName = kundo2_i18n("Subtract Shapes");
    }

    dstOutline = booleanWorkaroundTransform.inverted().map(dstOutline);

    KoShape *newShape = 0;

    if (!dstOutline.isEmpty()) {
        newShape = KoPathShape::createShapeFromPainterPath(dstOutline);
    }

    KUndo2Command *cmd = new KUndo2Command(actionName);

    new KoKeepShapesSelectedCommand(editableShapes, {}, canvas()->selectedShapesProxy(), false, cmd);

    QList<KoShape*> newSelectedShapes;

    if (newShape) {
        newShape->setBackground(referenceShape->background());
        newShape->setStroke(referenceShape->stroke());
        newShape->setZIndex(referenceShape->zIndex());

        KoShapeContainer *parent = referenceShape->parent();
        canvas()->shapeController()->addShapeDirect(newShape, parent, cmd);

        newSelectedShapes << newShape;
    }

    canvas()->shapeController()->removeShapes(editableShapes, cmd);

    new KoKeepShapesSelectedCommand({}, newSelectedShapes, canvas()->selectedShapesProxy(), true, cmd);

    canvas()->addCommand(cmd);
}

void DefaultTool::selectionSplitShapes()
{
    KoSelection *selection = koSelection();
    if (!selection) return;

    QList<KoShape *> editableShapes = selection->selectedEditableShapes();
    if (editableShapes.isEmpty()) {
        return;
    }

    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Split Shapes"));

    new KoKeepShapesSelectedCommand(editableShapes, {}, canvas()->selectedShapesProxy(), false, cmd);
    QList<KoShape*> newShapes;

    Q_FOREACH (KoShape *shape, editableShapes) {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
        if (!pathShape) return;

        QList<KoPathShape*> splitShapes;
        if (pathShape->separate(splitShapes)) {
            QList<KoShape*> normalShapes = implicitCastList<KoShape*>(splitShapes);

            KoShapeContainer *parent = shape->parent();
            canvas()->shapeController()->addShapesDirect(normalShapes, parent, cmd);
            canvas()->shapeController()->removeShape(shape, cmd);
            newShapes << normalShapes;
        }
    }

    new KoKeepShapesSelectedCommand({}, newShapes, canvas()->selectedShapesProxy(), true, cmd);

    canvas()->addCommand(cmd);
}

void DefaultTool::selectionAlign(int _align)
{
    KoShapeAlignCommand::Align align =
        static_cast<KoShapeAlignCommand::Align>(_align);

    KoSelection *selection = koSelection();
    if (!selection) return;

    QList<KoShape *> editableShapes = selection->selectedEditableShapes();
    if (editableShapes.isEmpty()) {
        return;
    }

    // TODO add an option to the widget so that one can align to the page
    // with multiple selected shapes too

    QRectF bb;

    // single selected shape is automatically aligned to document rect
    if (editableShapes.count() == 1) {
        if (!canvas()->resourceManager()->hasResource(KoCanvasResource::PageSize)) {
            return;
        }
        bb = QRectF(QPointF(0, 0), canvas()->resourceManager()->sizeResource(KoCanvasResource::PageSize));
    } else {
        bb = KoShape::absoluteOutlineRect(editableShapes);
    }

    KoShapeAlignCommand *cmd = new KoShapeAlignCommand(editableShapes, align, bb);
    canvas()->addCommand(cmd);
}

void DefaultTool::selectionDistribute(int _distribute)
{
    KoShapeDistributeCommand::Distribute distribute =
        static_cast<KoShapeDistributeCommand::Distribute>(_distribute);

    KoSelection *selection = koSelection();
    if (!selection) return;

    QList<KoShape *> editableShapes = selection->selectedEditableShapes();
    if (editableShapes.size() < 3) {
        return;
    }

    QRectF bb = KoShape::absoluteOutlineRect(editableShapes);
    KoShapeDistributeCommand *cmd = new KoShapeDistributeCommand(editableShapes, distribute, bb);
    canvas()->addCommand(cmd);
}

void DefaultTool::selectionBringToFront()
{
    selectionReorder(KoShapeReorderCommand::BringToFront);
}

void DefaultTool::selectionMoveUp()
{
    selectionReorder(KoShapeReorderCommand::RaiseShape);
}

void DefaultTool::selectionMoveDown()
{
    selectionReorder(KoShapeReorderCommand::LowerShape);
}

void DefaultTool::selectionSendToBack()
{
    selectionReorder(KoShapeReorderCommand::SendToBack);
}

void DefaultTool::selectionReorder(KoShapeReorderCommand::MoveShapeType order)
{
    KoSelection *selection = koSelection();
    if (!selection) {
        return;
    }

    QList<KoShape *> selectedShapes = selection->selectedEditableShapes();
    if (selectedShapes.isEmpty()) {
        return;
    }

    KUndo2Command *cmd = KoShapeReorderCommand::createCommand(selectedShapes, shapeManager(), order);
    if (cmd) {
        canvas()->addCommand(cmd);
    }
}

QList<QPointer<QWidget> > DefaultTool::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgets;

    m_tabbedOptionWidget = new DefaultToolTabbedWidget(this);

    if (isActivated()) {
        m_tabbedOptionWidget->activate();
    }
    widgets.append(m_tabbedOptionWidget);

    connect(m_tabbedOptionWidget,
            SIGNAL(sigSwitchModeEditFillGradient(bool)),
            SLOT(slotActivateEditFillGradient(bool)));

    connect(m_tabbedOptionWidget,
            SIGNAL(sigSwitchModeEditStrokeGradient(bool)),
            SLOT(slotActivateEditStrokeGradient(bool)));

    connect(m_tabbedOptionWidget,
            SIGNAL(sigSwitchModeEditFillGradient(bool)),
            SLOT(slotActivateEditFillMeshGradient(bool)));
    // TODO: strokes!!

    connect(m_tabbedOptionWidget,
            SIGNAL(sigMeshGradientResetted()),
            SLOT(slotResetMeshGradientState()));

    return widgets;
}

void DefaultTool::canvasResourceChanged(int key, const QVariant &res)
{
    if (key == HotPosition) {
        m_hotPosition = KoFlake::AnchorPosition(res.toInt());
        repaintDecorations();
    }
}

KoInteractionStrategy *DefaultTool::createStrategy(KoPointerEvent *event)
{
    KoSelection *selection = koSelection();
    if (!selection) return nullptr;

    bool insideSelection = false;
    KoFlake::SelectionHandle handle = handleAt(event->point, &insideSelection);

    bool editableShape = !selection->selectedEditableShapes().isEmpty();

    const bool selectMultiple = event->modifiers() & Qt::ShiftModifier;
    const bool selectNextInStack = event->modifiers() & Qt::ControlModifier;
    const bool avoidSelection = event->modifiers() & Qt::AltModifier;

    if (selectNextInStack) {
        // change the hot selection position when middle clicking on a handle
        KoFlake::AnchorPosition newHotPosition = m_hotPosition;
        switch (handle) {
        case KoFlake::TopMiddleHandle:
            newHotPosition = KoFlake::Top;
            break;
        case KoFlake::TopRightHandle:
            newHotPosition = KoFlake::TopRight;
            break;
        case KoFlake::RightMiddleHandle:
            newHotPosition = KoFlake::Right;
            break;
        case KoFlake::BottomRightHandle:
            newHotPosition = KoFlake::BottomRight;
            break;
        case KoFlake::BottomMiddleHandle:
            newHotPosition = KoFlake::Bottom;
            break;
        case KoFlake::BottomLeftHandle:
            newHotPosition = KoFlake::BottomLeft;
            break;
        case KoFlake::LeftMiddleHandle:
            newHotPosition = KoFlake::Left;
            break;
        case KoFlake::TopLeftHandle:
            newHotPosition = KoFlake::TopLeft;
            break;
        case KoFlake::NoHandle:
        default:
            // check if we had hit the center point
            const KoViewConverter *converter = canvas()->viewConverter();
            QPointF pt = converter->documentToView(event->point);

            // TODO: use calculated values instead!
            QPointF centerPt = converter->documentToView(selection->absolutePosition());

            if (kisSquareDistance(pt, centerPt) < HANDLE_DISTANCE_SQ) {
                newHotPosition = KoFlake::Center;
            }

            break;
        }

        if (m_hotPosition != newHotPosition) {
            canvas()->resourceManager()->setResource(HotPosition, newHotPosition);
            return new NopInteractionStrategy(this);
        }
    }

    if (!avoidSelection && editableShape) {
        // manipulation of selected shapes goes first
        if (handle != KoFlake::NoHandle) {
            // resizing or shearing only with left mouse button
            if (insideSelection) {
                bool forceUniformScaling = m_tabbedOptionWidget && m_tabbedOptionWidget->useUniformScaling();
                return new ShapeResizeStrategy(this, selection, event->point, handle, forceUniformScaling);
            }

            if (handle == KoFlake::TopMiddleHandle || handle == KoFlake::RightMiddleHandle ||
                handle == KoFlake::BottomMiddleHandle || handle == KoFlake::LeftMiddleHandle) {

                return new ShapeShearStrategy(this, selection, event->point, handle);
            }

            // rotating is allowed for right mouse button too
            if (handle == KoFlake::TopLeftHandle || handle == KoFlake::TopRightHandle ||
                    handle == KoFlake::BottomLeftHandle || handle == KoFlake::BottomRightHandle) {

                return new ShapeRotateStrategy(this, selection, event->point, event->buttons());
            }
        }

        if (!selectMultiple && !selectNextInStack) {

           if (insideSelection) {
                return new ShapeMoveStrategy(this, selection, event->point);
            }
        }
    }

    KoShape *shape = shapeManager()->shapeAt(event->point, selectNextInStack ? KoFlake::NextUnselected : KoFlake::ShapeOnTop);

    if (avoidSelection || (!shape && handle == KoFlake::NoHandle)) {
        if (!selectMultiple) {
            selection->deselectAll();
        }
        return new SelectionInteractionStrategy(this, event->point, false);
    }

    if (selection->isSelected(shape)) {
        if (selectMultiple) {
            selection->deselect(shape);
        }
    } else if (handle == KoFlake::NoHandle) { // clicked on shape which is not selected
        if (!selectMultiple) {
            selection->deselectAll();
        }
        selection->select(shape);
        // tablet selection isn't precise and may lead to a move, preventing that
        if (event->isTabletEvent()) {
            return new NopInteractionStrategy(this);
        }
        return new ShapeMoveStrategy(this, selection, event->point);
    }
    return 0;
}

void DefaultTool::updateActions()
{
    QList<KoShape*> editableShapes;

    if (koSelection()) {
        editableShapes = koSelection()->selectedEditableShapes();
    }

    const bool hasEditableShapes = !editableShapes.isEmpty();

    action("object_order_front")->setEnabled(hasEditableShapes);
    action("object_order_raise")->setEnabled(hasEditableShapes);
    action("object_order_lower")->setEnabled(hasEditableShapes);
    action("object_order_back")->setEnabled(hasEditableShapes);

    action("object_transform_rotate_90_cw")->setEnabled(hasEditableShapes);
    action("object_transform_rotate_90_ccw")->setEnabled(hasEditableShapes);
    action("object_transform_rotate_180")->setEnabled(hasEditableShapes);
    action("object_transform_mirror_horizontally")->setEnabled(hasEditableShapes);
    action("object_transform_mirror_vertically")->setEnabled(hasEditableShapes);
    action("object_transform_reset")->setEnabled(hasEditableShapes);

    const bool multipleSelected = editableShapes.size() > 1;

    const bool alignmentEnabled =
       multipleSelected ||
       (!editableShapes.isEmpty() &&
        canvas()->resourceManager()->hasResource(KoCanvasResource::PageSize));

    action("object_align_horizontal_left")->setEnabled(alignmentEnabled);
    action("object_align_horizontal_center")->setEnabled(alignmentEnabled);
    action("object_align_horizontal_right")->setEnabled(alignmentEnabled);
    action("object_align_vertical_top")->setEnabled(alignmentEnabled);
    action("object_align_vertical_center")->setEnabled(alignmentEnabled);
    action("object_align_vertical_bottom")->setEnabled(alignmentEnabled);

    const bool distributionEnabled = editableShapes.size() > 2;

    action("object_distribute_horizontal_left")->setEnabled(distributionEnabled);
    action("object_distribute_horizontal_center")->setEnabled(distributionEnabled);
    action("object_distribute_horizontal_right")->setEnabled(distributionEnabled);
    action("object_distribute_horizontal_gaps")->setEnabled(distributionEnabled);

    action("object_distribute_vertical_top")->setEnabled(distributionEnabled);
    action("object_distribute_vertical_center")->setEnabled(distributionEnabled);
    action("object_distribute_vertical_bottom")->setEnabled(distributionEnabled);
    action("object_distribute_vertical_gaps")->setEnabled(distributionEnabled);

    updateDistinctiveActions(editableShapes);

    emit selectionChanged(editableShapes.size());
}

void DefaultTool::updateDistinctiveActions(const QList<KoShape*> &editableShapes) {
    const bool multipleSelected = editableShapes.size() > 1;

    action("object_group")->setEnabled(multipleSelected);

    action("object_unite")->setEnabled(multipleSelected);
    action("object_intersect")->setEnabled(multipleSelected);
    action("object_subtract")->setEnabled(multipleSelected);

    bool hasShapesWithMultipleSegments = false;
    Q_FOREACH (KoShape *shape, editableShapes) {
            KoPathShape *pathShape = dynamic_cast<KoPathShape *>(shape);
            if (pathShape && pathShape->subpathCount() > 1) {
                hasShapesWithMultipleSegments = true;
                break;
            }
        }
    action("object_split")->setEnabled(hasShapesWithMultipleSegments);


    bool hasGroupShape = false;
            foreach (KoShape *shape, editableShapes) {
            if (dynamic_cast<KoShapeGroup *>(shape)) {
                hasGroupShape = true;
                break;
            }
        }
    action("object_ungroup")->setEnabled(hasGroupShape);
}


KoToolSelection *DefaultTool::selection()
{
    return m_selectionHandler;
}

QMenu* DefaultTool::popupActionsMenu()
{
    if (m_contextMenu) {
        m_contextMenu->clear();

        m_contextMenu->addSection(i18n("Vector Shape Actions"));
        m_contextMenu->addSeparator();

        QMenu *transform = m_contextMenu->addMenu(i18n("Transform"));

        transform->addAction(action("object_transform_rotate_90_cw"));
        transform->addAction(action("object_transform_rotate_90_ccw"));
        transform->addAction(action("object_transform_rotate_180"));
        transform->addSeparator();
        transform->addAction(action("object_transform_mirror_horizontally"));
        transform->addAction(action("object_transform_mirror_vertically"));
        transform->addSeparator();
        transform->addAction(action("object_transform_reset"));

        if (action("object_unite")->isEnabled() ||
            action("object_intersect")->isEnabled() ||
            action("object_subtract")->isEnabled() ||
            action("object_split")->isEnabled()) {

            QMenu *transform = m_contextMenu->addMenu(i18n("Logical Operations"));
            transform->addAction(action("object_unite"));
            transform->addAction(action("object_intersect"));
            transform->addAction(action("object_subtract"));
            transform->addAction(action("object_split"));
        }

        m_contextMenu->addSeparator();

        m_contextMenu->addAction(action("edit_cut"));
        m_contextMenu->addAction(action("edit_copy"));
        m_contextMenu->addAction(action("edit_paste"));

        m_contextMenu->addSeparator();

        m_contextMenu->addAction(action("object_order_front"));
        m_contextMenu->addAction(action("object_order_raise"));
        m_contextMenu->addAction(action("object_order_lower"));
        m_contextMenu->addAction(action("object_order_back"));

        if (action("object_group")->isEnabled() || action("object_ungroup")->isEnabled()) {
            m_contextMenu->addSeparator();
            m_contextMenu->addAction(action("object_group"));
            m_contextMenu->addAction(action("object_ungroup"));
        }
    }

    return m_contextMenu.data();
}

void DefaultTool::addTransformActions(QMenu *menu) const {
    menu->addAction(action("object_transform_rotate_90_cw"));
    menu->addAction(action("object_transform_rotate_90_ccw"));
    menu->addAction(action("object_transform_rotate_180"));
    menu->addSeparator();
    menu->addAction(action("object_transform_mirror_horizontally"));
    menu->addAction(action("object_transform_mirror_vertically"));
    menu->addSeparator();
    menu->addAction(action("object_transform_reset"));
}

void DefaultTool::explicitUserStrokeEndRequest()
{
    QList<KoShape *> shapes = koSelection()->selectedEditableShapesAndDelegates();
    KoToolManager::instance()->switchToolRequested(KoToolManager::instance()->preferredToolForSelection(shapes));
}
