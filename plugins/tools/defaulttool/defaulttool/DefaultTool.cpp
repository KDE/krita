/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2008 C. Boemann <cbo@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
#include <KoShapeOdfSaveHelper.h>
#include <KoDrag.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceManager.h>
#include <KoShapeRubberSelectStrategy.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeDeleteCommand.h>
#include <commands/KoShapeCreateCommand.h>
#include <commands/KoShapeGroupCommand.h>
#include <commands/KoShapeUngroupCommand.h>
#include <commands/KoShapeDistributeCommand.h>
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

#include <QPointer>
#include <QAction>
#include <QKeyEvent>
#include <QSignalMapper>
#include <KoResourcePaths.h>

#include <KoCanvasController.h>
#include <kactioncollection.h>
#include <QMenu>

#include <math.h>
#include "kis_assert.h"
#include "kis_global.h"
#include "kis_debug.h"

#include <QVector2D>

#define HANDLE_DISTANCE 10
#define HANDLE_DISTANCE_SQ (HANDLE_DISTANCE * HANDLE_DISTANCE)

#define INNER_HANDLE_DISTANCE_SQ 16

namespace {
static const QString EditFillGradientFactoryId = "edit_fill_gradient";
static const QString EditStrokeGradientFactoryId = "edit_stroke_gradient";
}

QPolygonF selectionPolygon(KoSelection *selection)
{
    QPolygonF result;

    QList<KoShape*> selectedShapes = selection->selectedShapes();

    if (!selectedShapes.size()) {
        return result;
    }

    if (selectedShapes.size() > 1) {
        QTransform matrix = selection->absoluteTransformation(0);
        result = matrix.map(QPolygonF(QRectF(QPointF(0, 0), selection->size())));
    } else {
        KoShape *selectedShape = selectedShapes.first();
        QTransform matrix = selectedShape->absoluteTransformation(0);
        result = matrix.map(QPolygonF(QRectF(QPointF(0, 0), selectedShape->size())));
    }

    return result;
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
        }

        return m_currentHandle.type != KoShapeGradientHandles::Handle::None;
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

DefaultTool::DefaultTool(KoCanvasBase *canvas)
    : KoInteractionTool(canvas)
    , m_lastHandle(KoFlake::NoHandle)
    , m_hotPosition(KoFlake::TopLeft)
    , m_mouseWasInsideHandles(false)
    , m_selectionHandler(new SelectionHandler(this))
    , m_customEventStrategy(0)
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

    connect(canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(updateActions()));
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

bool DefaultTool::wantsAutoScroll() const
{
    return true;
}

void DefaultTool::addMappedAction(QSignalMapper *mapper, const QString &actionId, int commandType)
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();

    QAction *action = actionRegistry->makeQAction(actionId, this);
    addAction(actionId, action);
    connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
    mapper->setMapping(action, commandType);
}

void DefaultTool::setupActions()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();

    QAction *actionBringToFront = actionRegistry->makeQAction("object_order_front", this);
    addAction("object_order_front", actionBringToFront);
    connect(actionBringToFront, SIGNAL(triggered()), this, SLOT(selectionBringToFront()));

    QAction *actionRaise = actionRegistry->makeQAction("object_order_raise", this);
    addAction("object_order_raise", actionRaise);
    connect(actionRaise, SIGNAL(triggered()), this, SLOT(selectionMoveUp()));

    QAction *actionLower = actionRegistry->makeQAction("object_order_lower", this);
    addAction("object_order_lower", actionLower);
    connect(actionLower, SIGNAL(triggered()), this, SLOT(selectionMoveDown()));

    QAction *actionSendToBack = actionRegistry->makeQAction("object_order_back", this);
    addAction("object_order_back", actionSendToBack);
    connect(actionSendToBack, SIGNAL(triggered()), this, SLOT(selectionSendToBack()));


    QSignalMapper *alignSignalsMapper = new QSignalMapper(this);
    connect(alignSignalsMapper, SIGNAL(mapped(int)), SLOT(selectionAlign(int)));

    addMappedAction(alignSignalsMapper, "object_align_horizontal_left", KoShapeAlignCommand::HorizontalLeftAlignment);
    addMappedAction(alignSignalsMapper, "object_align_horizontal_center", KoShapeAlignCommand::HorizontalCenterAlignment);
    addMappedAction(alignSignalsMapper, "object_align_horizontal_right", KoShapeAlignCommand::HorizontalRightAlignment);
    addMappedAction(alignSignalsMapper, "object_align_vertical_top", KoShapeAlignCommand::VerticalTopAlignment);
    addMappedAction(alignSignalsMapper, "object_align_vertical_center", KoShapeAlignCommand::VerticalCenterAlignment);
    addMappedAction(alignSignalsMapper, "object_align_vertical_bottom", KoShapeAlignCommand::VerticalBottomAlignment);

    QSignalMapper *distributeSignalsMapper = new QSignalMapper(this);
    connect(distributeSignalsMapper, SIGNAL(mapped(int)), SLOT(selectionDistribute(int)));

    addMappedAction(distributeSignalsMapper, "object_distribute_horizontal_left", KoShapeDistributeCommand::HorizontalLeftDistribution);
    addMappedAction(distributeSignalsMapper, "object_distribute_horizontal_center", KoShapeDistributeCommand::HorizontalCenterDistribution);
    addMappedAction(distributeSignalsMapper, "object_distribute_horizontal_right", KoShapeDistributeCommand::HorizontalRightDistribution);
    addMappedAction(distributeSignalsMapper, "object_distribute_horizontal_gaps", KoShapeDistributeCommand::HorizontalGapsDistribution);

    addMappedAction(distributeSignalsMapper, "object_distribute_vertical_top", KoShapeDistributeCommand::VerticalTopDistribution);
    addMappedAction(distributeSignalsMapper, "object_distribute_vertical_center", KoShapeDistributeCommand::VerticalCenterDistribution);
    addMappedAction(distributeSignalsMapper, "object_distribute_vertical_bottom", KoShapeDistributeCommand::VerticalBottomDistribution);
    addMappedAction(distributeSignalsMapper, "object_distribute_vertical_gaps", KoShapeDistributeCommand::VerticalGapsDistribution);

    QAction *actionGroupBottom = actionRegistry->makeQAction("object_group", this);
    addAction("object_group", actionGroupBottom);
    connect(actionGroupBottom, SIGNAL(triggered()), this, SLOT(selectionGroup()));

    QAction *actionUngroupBottom = actionRegistry->makeQAction("object_ungroup", this);
    addAction("object_ungroup", actionUngroupBottom);
    connect(actionUngroupBottom, SIGNAL(triggered()), this, SLOT(selectionUngroup()));

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
    case KoFlake::NoHandle:
        break;
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

    if (koSelection()->count() > 0) { // has a selection
        bool editable = !koSelection()->selectedEditableShapes().isEmpty();

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
    // this tool only works on a vector layer right now, so give a warning if another layer type is trying to use it
    KisNodeSP currentNode = canvas()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeWSP>();

    if (currentNode.isNull() || !currentNode->inherits("KisShapeLayer")) {

        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        kiscanvas->viewManager()->showFloatingMessage(
            i18n("This tool only works on vector layers. You probably want the move tool."),
            QIcon(), 2000, KisFloatingMessage::Medium, Qt::AlignCenter);

        return;
    }


    SelectionDecorator decorator(canvas()->resourceManager());
    decorator.setSelection(koSelection());
    decorator.setHandleRadius(handleRadius());
    decorator.setShowFillGradientHandles(hasInteractioFactory(EditFillGradientFactoryId));
    decorator.setShowStrokeFillGradientHandles(hasInteractioFactory(EditStrokeGradientFactoryId));
    decorator.paint(painter, converter);

    KoInteractionTool::paint(painter, converter);

    painter.save();
    KoShape::applyConversion(painter, converter);
    canvas()->snapGuide()->paint(painter, converter);
    painter.restore();
}

void DefaultTool::mousePressEvent(KoPointerEvent *event)
{
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
                //repaintDecorations();
            }
        } else {
            /*if (m_lastHandle != KoFlake::NoHandle)
                repaintDecorations(); */
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
    if (!selection->count()) return QRectF();

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
    KoSelection *selection = canvas()->selectedShapesProxy()->selection();

    KoShape *shape = canvas()->shapeManager()->shapeAt(event->point, KoFlake::ShapeOnTop);
    if (shape && !selection->isSelected(shape)) {

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

void DefaultTool::repaintDecorations()
{
    if (koSelection() && koSelection()->count() > 0) {
        canvas()->updateCanvas(handlesSize());
    }
}

void DefaultTool::copy() const
{
    // all the selected shapes, not only editable!
    QList<KoShape *> shapes = canvas()->selectedShapesProxy()->selection()->selectedShapes();

    if (!shapes.isEmpty()) {
        KoDrag drag;
        drag.setSvg(shapes);
        drag.addToClipboard();
    }
}

void DefaultTool::deleteSelection()
{
    QList<KoShape *> shapes;
    foreach (KoShape *s, canvas()->selectedShapesProxy()->selection()->selectedShapes()) {
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

KoSelection *DefaultTool::koSelection()
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

    if (!selection->count() || !converter) {
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

    QTransform matrix = selection->absoluteTransformation(0);
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

void DefaultTool::activate(ToolActivation activation, const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(activation, shapes);

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

    KoShapeGroup *group = new KoShapeGroup();
    // TODO what if only one shape is left?
    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Group shapes"));
    canvas()->shapeController()->addShapeDirect(group, cmd);
    new KoShapeGroupCommand(group, selectedShapes, false, true, true, cmd);
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

    // add a ungroup command for each found shape container to the macro command
    Q_FOREACH (KoShape *shape, selectedShapes) {
        KoShapeGroup *group = dynamic_cast<KoShapeGroup *>(shape);
        if (group) {
            cmd = cmd ? cmd : new KUndo2Command(kundo2_i18n("Ungroup shapes"));
            new KoShapeUngroupCommand(group, group->shapes(),
                                      group->parent() ? QList<KoShape *>() : canvas()->shapeManager()->topLevelShapes(),
                                      cmd);
            canvas()->shapeController()->removeShape(group, cmd);
        }
    }
    if (cmd) {
        canvas()->addCommand(cmd);
    }
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
        if (!canvas()->resourceManager()->hasResource(KoCanvasResourceManager::PageSize)) {
            return;
        }
        bb = QRectF(QPointF(0, 0), canvas()->resourceManager()->sizeResource(KoCanvasResourceManager::PageSize));
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
    KoSelection *selection = canvas()->selectedShapesProxy()->selection();
    if (!selection) {
        return;
    }

    QList<KoShape *> selectedShapes = selection->selectedEditableShapes();
    if (selectedShapes.isEmpty()) {
        return;
    }

    KUndo2Command *cmd = KoShapeReorderCommand::createCommand(selectedShapes, canvas()->shapeManager(), order);
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
    KoShapeManager *shapeManager = canvas()->shapeManager();
    KoSelection *selection = koSelection();

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
                return new ShapeResizeStrategy(this, event->point, handle);
            }

            if (handle == KoFlake::TopMiddleHandle || handle == KoFlake::RightMiddleHandle ||
                handle == KoFlake::BottomMiddleHandle || handle == KoFlake::LeftMiddleHandle) {

                return new ShapeShearStrategy(this, event->point, handle);
            }

            // rotating is allowed for rigth mouse button too
            if (handle == KoFlake::TopLeftHandle || handle == KoFlake::TopRightHandle ||
                    handle == KoFlake::BottomLeftHandle || handle == KoFlake::BottomRightHandle) {

                return new ShapeRotateStrategy(this, event->point, event->buttons());
            }
        }

        if (!selectMultiple && !selectNextInStack) {
            if (insideSelection) {
                return new ShapeMoveStrategy(this, event->point);
            }
        }
    }

    KoShape *shape = shapeManager->shapeAt(event->point, selectNextInStack ? KoFlake::NextUnselected : KoFlake::ShapeOnTop);

    if (avoidSelection || (!shape && handle == KoFlake::NoHandle)) {
        if (!selectMultiple) {
            repaintDecorations();
            selection->deselectAll();
        }
        return new SelectionInteractionStrategy(this, event->point, false);
    }

    if (selection->isSelected(shape)) {
        if (selectMultiple) {
            repaintDecorations();
            selection->deselect(shape);
        }
    } else if (handle == KoFlake::NoHandle) { // clicked on shape which is not selected
        repaintDecorations();
        if (!selectMultiple) {
            shapeManager->selection()->deselectAll();
        }
        selection->select(shape);
        repaintDecorations();
        // tablet selection isn't precise and may lead to a move, preventing that
        if (event->isTabletEvent()) {
            return new NopInteractionStrategy(this);
        }
        return new ShapeMoveStrategy(this, event->point);
    }
    return 0;
}

void DefaultTool::updateActions()
{
    QList<KoShape*> editableShapes;

    if (koSelection()) {
        editableShapes = koSelection()->selectedEditableShapes();
    }

    const bool orderingEnabled = !editableShapes.isEmpty();

    action("object_order_front")->setEnabled(orderingEnabled);
    action("object_order_raise")->setEnabled(orderingEnabled);
    action("object_order_lower")->setEnabled(orderingEnabled);
    action("object_order_back")->setEnabled(orderingEnabled);

    const bool alignmentEnabled =
       editableShapes.size() > 1 ||
       (!editableShapes.isEmpty() &&
        canvas()->resourceManager()->hasResource(KoCanvasResourceManager::PageSize));

    action("object_align_horizontal_left")->setEnabled(alignmentEnabled);
    action("object_align_horizontal_center")->setEnabled(alignmentEnabled);
    action("object_align_horizontal_right")->setEnabled(alignmentEnabled);
    action("object_align_vertical_top")->setEnabled(alignmentEnabled);
    action("object_align_vertical_center")->setEnabled(alignmentEnabled);
    action("object_align_vertical_bottom")->setEnabled(alignmentEnabled);

    action("object_group")->setEnabled(editableShapes.size() > 1);

    const bool distributionEnabled = editableShapes.size() > 2;

    action("object_distribute_horizontal_left")->setEnabled(distributionEnabled);
    action("object_distribute_horizontal_center")->setEnabled(distributionEnabled);
    action("object_distribute_horizontal_right")->setEnabled(distributionEnabled);
    action("object_distribute_horizontal_gaps")->setEnabled(distributionEnabled);

    action("object_distribute_vertical_top")->setEnabled(distributionEnabled);
    action("object_distribute_vertical_center")->setEnabled(distributionEnabled);
    action("object_distribute_vertical_bottom")->setEnabled(distributionEnabled);
    action("object_distribute_vertical_gaps")->setEnabled(distributionEnabled);


    bool hasGroupShape = false;
    foreach (KoShape *shape, editableShapes) {
        if (dynamic_cast<KoShapeGroup *>(shape)) {
            hasGroupShape = true;
            break;
        }
    }
    action("object_ungroup")->setEnabled(hasGroupShape);

    emit selectionChanged(editableShapes.size());
}

KoToolSelection *DefaultTool::selection()
{
    return m_selectionHandler;
}

QMenu* DefaultTool::popupActionsMenu()
{
    if (m_contextMenu) {
        m_contextMenu->clear();

        KActionCollection *collection = this->canvas()->canvasController()->actionCollection();

        m_contextMenu->addAction(collection->action("edit_cut"));
        m_contextMenu->addAction(collection->action("edit_copy"));
        m_contextMenu->addAction(collection->action("edit_paste"));

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

void DefaultTool::explicitUserStrokeEndRequest()
{
    QList<KoShape *> shapes = koSelection()->selectedEditableShapesAndDelegates();
    emit activateTemporary(KoToolManager::instance()->preferredToolForSelection(shapes));
}
