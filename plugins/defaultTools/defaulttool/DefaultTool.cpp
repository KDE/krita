/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>

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
#include "DefaultToolWidget.h"
#include "DefaultToolArrangeWidget.h"
#include "SelectionDecorator.h"
#include "ShapeMoveStrategy.h"
#include "ShapeRotateStrategy.h"
#include "ShapeShearStrategy.h"
#include "ShapeResizeStrategy.h"
#include "guidestool/GuidesTool.h"
#include "guidestool/GuidesToolFactory.h" // for the ID

#include <KoPointerEvent.h>
#include <KoToolSelection.h>
#include <KoToolManager.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoShapeGroup.h>
#include <KoShapeLayer.h>
#include <KoShapePaste.h>
#include <KoShapeOdfSaveHelper.h>
#include <KoDrag.h>
#include <KoDocument.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoShapeRubberSelectStrategy.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeDeleteCommand.h>
#include <commands/KoShapeCreateCommand.h>
#include <commands/KoShapeGroupCommand.h>
#include <commands/KoShapeUngroupCommand.h>
#include <KoSnapGuide.h>

#include <KAction>
#include <QKeyEvent>
#include <QClipboard>
#include <kstandarddirs.h>

#include <math.h>

#define HANDLE_DISTANCE 10

class SelectionHandler : public KoToolSelection
{
public:
    SelectionHandler(DefaultTool *parent)
        : KoToolSelection(parent), m_selection(parent->koSelection())
    {
        Q_ASSERT(m_selection);
    }

    bool hasSelection() {
        return m_selection->count();
    }

private:
    KoSelection *m_selection;
};

class DefaultTool::GuideLine
{
public:
    GuideLine()
        : m_orientation(Qt::Horizontal), m_index(0), m_valid(false), m_selected(false)
    {
    }
    GuideLine(Qt::Orientation orientation, uint index)
        : m_orientation(orientation), m_index(index), m_valid(true), m_selected(false)
    {
    }

    bool isValid() const
    {
        return m_valid;
    }
    bool isSelected() const
    {
        return m_selected;
    }
    void select()
    {
        m_selected = true;
    }

    uint index() const
    {
        return m_index;
    }
    Qt::Orientation orientation() const
    {
        return m_orientation;
    }
private:
    Qt::Orientation m_orientation;
    uint m_index;
    bool m_valid;
    bool m_selected;
};


DefaultTool::DefaultTool(KoCanvasBase *canvas)
    : KoInteractionTool(canvas),
    m_lastHandle(KoFlake::NoHandle),
    m_hotPosition(KoFlake::TopLeftCorner),
    m_mouseWasInsideHandles(false),
    m_moveCommand(0),
    m_selectionHandler(new SelectionHandler(this)),
    m_customEventStrategy(0),
    m_guideLine(new GuideLine())
{
    setupActions();

    QPixmap rotatePixmap, shearPixmap;
    rotatePixmap.load(KStandardDirs::locate("data", "koffice/icons/rotate.png"));
    shearPixmap.load(KStandardDirs::locate("data", "koffice/icons/shear.png"));

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

    KoShapeManager * manager = canvas->shapeManager();
    connect(manager, SIGNAL(selectionChanged()), this, SLOT(updateActions()));
}

DefaultTool::~DefaultTool()
{
    delete m_guideLine;
}

bool DefaultTool::wantsAutoScroll() const
{
    return true;
}

void DefaultTool::setupActions()
{
    KAction* actionBringToFront = new KAction(KIcon("object-order-front-koffice"),
                                               i18n("Bring to &Front"), this);
    addAction("object_order_front", actionBringToFront);
    actionBringToFront->setShortcut(QKeySequence("Ctrl+Shift+]"));
    connect(actionBringToFront, SIGNAL(triggered()), this, SLOT(selectionBringToFront()));

    KAction* actionRaise = new KAction(KIcon("object-order-raise-koffice"), i18n("&Raise"), this);
    addAction("object_order_raise", actionRaise);
    actionRaise->setShortcut(QKeySequence("Ctrl+]"));
    connect(actionRaise, SIGNAL(triggered()), this, SLOT(selectionMoveUp()));

    KAction* actionLower = new KAction(KIcon("object-order-lower-koffice"), i18n("&Lower"), this);
    addAction("object_order_lower", actionLower);
    actionLower->setShortcut(QKeySequence("Ctrl+["));
    connect(actionLower, SIGNAL(triggered()), this, SLOT(selectionMoveDown()));

    KAction* actionSendToBack = new KAction(KIcon("object-order-back-koffice"),
                                             i18n("Send to &Back"), this);
    addAction("object_order_back", actionSendToBack);
    actionSendToBack->setShortcut(QKeySequence("Ctrl+Shift+["));
    connect(actionSendToBack, SIGNAL(triggered()), this, SLOT(selectionSendToBack()));

    KAction* actionAlignLeft = new KAction(KIcon("object-align-horizontal-left-koffice"),
                                            i18n("Align Left"), this);
    addAction("object_align_horizontal_left", actionAlignLeft);
    connect(actionAlignLeft, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalLeft()));

    KAction* actionAlignCenter = new KAction(KIcon("object-align-horizontal-center-koffice"),
                                              i18n("Horizontally Center"), this);
    addAction("object_align_horizontal_center", actionAlignCenter);
    connect(actionAlignCenter, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalCenter()));

    KAction* actionAlignRight = new KAction(KIcon("object-align-horizontal-right-koffice"),
                                             i18n("Align Right"), this);
    addAction("object_align_horizontal_right", actionAlignRight);
    connect(actionAlignRight, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalRight()));

    KAction* actionAlignTop = new KAction(KIcon("object-align-vertical-top-koffice"), i18n("Align Top"), this);
    addAction("object_align_vertical_top", actionAlignTop);
    connect(actionAlignTop, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalTop()));

    KAction* actionAlignMiddle = new KAction(KIcon("object-align-vertical-center-koffice"),
                                              i18n("Vertically Center"), this);
    addAction("object_align_vertical_center", actionAlignMiddle);
    connect(actionAlignMiddle, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalCenter()));

    KAction* actionAlignBottom = new KAction(KIcon("object-align-vertical-bottom-koffice"),
                                              i18n("Align Bottom"), this);
    addAction("object_align_vertical_bottom", actionAlignBottom);
    connect(actionAlignBottom, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalBottom()));

    KAction* actionGroupBottom = new KAction(KIcon("object-group-koffice"),
                                              i18n("Group"), this);
    addAction("object_group", actionGroupBottom);
    connect(actionGroupBottom, SIGNAL(triggered()), this, SLOT(selectionGroup()));

    KAction* actionUngroupBottom = new KAction(KIcon("object-ungroup-koffice"),
                                                i18n("Ungroup"), this);
    addAction("object_ungroup", actionUngroupBottom);
    connect(actionUngroupBottom, SIGNAL(triggered()), this, SLOT(selectionUngroup()));
}

qreal DefaultTool::rotationOfHandle(KoFlake::SelectionHandle handle, bool useEdgeRotation)
{
    QPointF selectionCenter = koSelection()->absolutePosition();
    QPointF direction;

    switch (handle) {
    case KoFlake::TopMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::TopRightCorner)
                - koSelection()->absolutePosition(KoFlake::TopLeftCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::TopLeftCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::TopRightCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::TopRightHandle:
        direction = koSelection()->absolutePosition(KoFlake::TopRightCorner) - selectionCenter;
        break;
    case KoFlake::RightMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::BottomRightCorner)
                    - koSelection()->absolutePosition(KoFlake::TopRightCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::TopRightCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::BottomRightCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::BottomRightHandle:
        direction = koSelection()->absolutePosition(KoFlake::BottomRightCorner) - selectionCenter;
        break;
    case KoFlake::BottomMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::BottomLeftCorner)
                    - koSelection()->absolutePosition(KoFlake::BottomRightCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::BottomLeftCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::BottomRightCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::BottomLeftHandle:
        direction = koSelection()->absolutePosition(KoFlake::BottomLeftCorner) - selectionCenter;
        break;
    case KoFlake::LeftMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KoFlake::TopLeftCorner)
                    - koSelection()->absolutePosition(KoFlake::BottomLeftCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KoFlake::TopLeftCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KoFlake::BottomLeftCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KoFlake::TopLeftHandle:
        direction = koSelection()->absolutePosition(KoFlake::TopLeftCorner) - selectionCenter;
        break;
    case KoFlake::NoHandle:
        return 0.0;
        break;
    }

    qreal rotation = atan2(direction.y(), direction.x()) * 180.0 / M_PI;

    switch (handle) {
    case KoFlake::TopMiddleHandle:
        if (useEdgeRotation)
            rotation -= 0.0;
        else
            rotation -= 270.0;
        break;
    case KoFlake::TopRightHandle:
        rotation -= 315.0;
        break;
    case KoFlake::RightMiddleHandle:
        if (useEdgeRotation)
            rotation -= 90.0;
        else
            rotation -= 0.0;
        break;
    case KoFlake::BottomRightHandle:
        rotation -= 45.0;
        break;
    case KoFlake::BottomMiddleHandle:
        if (useEdgeRotation)
            rotation -= 180.0;
        else
            rotation -= 90.0;
        break;
    case KoFlake::BottomLeftHandle:
        rotation -= 135.0;
        break;
    case KoFlake::LeftMiddleHandle:
        if (useEdgeRotation)
            rotation -= 270.0;
        else
            rotation -= 180.0;
        break;
    case KoFlake::TopLeftHandle:
        rotation -= 225.0;
        break;
    case KoFlake::NoHandle:
        break;
    }

    if (rotation < 0.0)
        rotation += 360.0;

    return rotation;
}

void DefaultTool::updateCursor()
{
    QCursor cursor = Qt::ArrowCursor;

    QString statusText;

    if (koSelection()->count() > 0) { // has a selection
        bool editable=editableShapesCount(koSelection()->selectedShapes(KoFlake::StrippedSelection));

        if (!m_mouseWasInsideHandles) {
            m_angle = rotationOfHandle(m_lastHandle, true);
            int rotOctant = 8 + int(8.5 + m_angle / 45);

            bool rotateHandle = false;
            bool shearHandle = false;
            switch(m_lastHandle) {
            case KoFlake::TopMiddleHandle:
                cursor = m_shearCursors[(0 +rotOctant)%8];
                shearHandle = true;
                break;
            case KoFlake::TopRightHandle:
                cursor = m_rotateCursors[(1 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KoFlake::RightMiddleHandle:
                cursor = m_shearCursors[(2 +rotOctant)%8];
                shearHandle = true;
                break;
            case KoFlake::BottomRightHandle:
                cursor = m_rotateCursors[(3 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KoFlake::BottomMiddleHandle:
                cursor = m_shearCursors[(4 +rotOctant)%8];
                shearHandle = true;
                break;
            case KoFlake::BottomLeftHandle:
                cursor = m_rotateCursors[(5 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KoFlake::LeftMiddleHandle:
                cursor = m_shearCursors[(6 +rotOctant)%8];
                shearHandle = true;
                break;
            case KoFlake::TopLeftHandle:
                cursor = m_rotateCursors[(7 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KoFlake::NoHandle:
                if (m_guideLine->isValid()) {
                    cursor = m_guideLine->orientation() == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor;
                    statusText = i18n("Click and drag to move guide line.");
                }
                else
                    cursor = Qt::ArrowCursor;
                break;
            }
            if (rotateHandle)
                statusText = i18n("Left click rotates around center, right click around highlighted position.");
            if (shearHandle)
                statusText = i18n("Click and drag to shear selection.");
        }
        else {
            statusText = i18n("Click and drag to resize selection.");
            m_angle = rotationOfHandle(m_lastHandle, false);
            int rotOctant = 8 + int(8.5 + m_angle / 45);
            bool cornerHandle = false;
            switch(m_lastHandle) {
            case KoFlake::TopMiddleHandle:
                cursor = m_sizeCursors[(0 +rotOctant)%8];
                break;
            case KoFlake::TopRightHandle:
                cursor = m_sizeCursors[(1 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KoFlake::RightMiddleHandle:
                cursor = m_sizeCursors[(2 +rotOctant)%8];
                break;
            case KoFlake::BottomRightHandle:
                cursor = m_sizeCursors[(3 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KoFlake::BottomMiddleHandle:
                cursor = m_sizeCursors[(4 +rotOctant)%8];
                break;
            case KoFlake::BottomLeftHandle:
                cursor = m_sizeCursors[(5 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KoFlake::LeftMiddleHandle:
                cursor = m_sizeCursors[(6 +rotOctant)%8];
                break;
            case KoFlake::TopLeftHandle:
                cursor = m_sizeCursors[(7 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KoFlake::NoHandle:
                cursor = Qt::SizeAllCursor;
                statusText = i18n("Click and drag to move selection.");
                break;
            }
            if (cornerHandle)
                statusText = i18n("Click and drag to resize selection. Middle click to set highlighted position.");
        }
        if (!editable)
            cursor = Qt::ArrowCursor;
    }
    else {
        if (m_guideLine->isValid()) {
            cursor = m_guideLine->orientation() == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor;
            statusText = i18n("Click and drag to move guide line.");
        }
    }
    useCursor(cursor);
    if (currentStrategy() == 0)
        emit statusTextChanged(statusText);
}

void DefaultTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    KoInteractionTool::paint(painter, converter);
    if (currentStrategy() == 0 && koSelection()->count() > 0) {
        SelectionDecorator decorator(m_mouseWasInsideHandles ? m_lastHandle : KoFlake::NoHandle,
                 true, true);
        decorator.setSelection(koSelection());
        decorator.setHandleRadius(canvas()->resourceManager()->handleRadius());
        decorator.setHotPosition(m_hotPosition);
        decorator.paint(painter, converter);
    }
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
    if (currentStrategy() == 0 && koSelection()->count() > 0) {
        QRectF bound = handlesSize();
        if (bound.contains(event->point)) {
            bool inside;
            KoFlake::SelectionHandle newDirection = handleAt(event->point, &inside);
            if (inside != m_mouseWasInsideHandles || m_lastHandle != newDirection) {
                m_lastHandle = newDirection;
                m_mouseWasInsideHandles = inside;
                repaintDecorations();
            }
        } else {
            if (m_lastHandle != KoFlake::NoHandle)
                repaintDecorations();
            m_lastHandle = KoFlake::NoHandle;
            m_mouseWasInsideHandles = false;

            if (m_guideLine->isSelected()) {
                GuidesTool *guidesTool = dynamic_cast<GuidesTool*>(KoToolManager::instance()->toolById(canvas(), GuidesToolId));
                if (guidesTool) {
                    guidesTool->moveGuideLine(m_guideLine->orientation(), m_guideLine->index());
                    activateTemporary(guidesTool->toolId());
                }
            } else {
                selectGuideAtPosition(event->point);
            }
        }
    } else {
        if (m_guideLine->isSelected()) {
            GuidesTool *guidesTool = dynamic_cast<GuidesTool*>(KoToolManager::instance()->toolById(canvas(), GuidesToolId));
            if (guidesTool) {
                guidesTool->moveGuideLine(m_guideLine->orientation(), m_guideLine->index());
                activateTemporary(guidesTool->toolId());
            }
        } else {
            selectGuideAtPosition(event->point);
        }
    }

    updateCursor();
}

void DefaultTool::selectGuideAtPosition(const QPointF &position)
{
    int index = -1;
    Qt::Orientation orientation = Qt::Horizontal;

    // check if we are on a guide line
    KoGuidesData * guidesData = canvas()->guidesData();
    if (guidesData && guidesData->showGuideLines()) {
        qreal grabSensitivity = canvas()->resourceManager()->grabSensitivity();
        qreal minDistance = canvas()->viewConverter()->viewToDocumentX(grabSensitivity);
        uint i = 0;
        foreach (qreal guidePos, guidesData->horizontalGuideLines()) {
            qreal distance = qAbs(guidePos - position.y());
            if (distance < minDistance) {
                orientation = Qt::Horizontal;
                index = i;
                minDistance = distance;
            }
            i++;
        }
        i = 0;
        foreach (qreal guidePos, guidesData->verticalGuideLines())
        {
            qreal distance = qAbs(guidePos - position.x());
            if (distance < minDistance) {
                orientation = Qt::Vertical;
                index = i;
                minDistance = distance;
            }
            i++;
        }
    }

    delete m_guideLine;
    if (index >= 0)
        m_guideLine = new GuideLine(orientation, index);
    else
        m_guideLine = new GuideLine();
}

QRectF DefaultTool::handlesSize()
{
    QRectF bound = koSelection()->boundingRect();
    // expansion Border
    if (!canvas() || !canvas()->viewConverter()) return bound;

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
    QList<KoShape*> shapes;
    foreach(KoShape *shape, koSelection()->selectedShapes()) {
        if (shape->boundingRect().contains(event->point) && // first 'cheap' check
                shape->outline().contains(event->point)) // this is more expensive but weeds out the almost hits
            shapes.append(shape);
    }
    if (shapes.count() == 0) { // nothing in the selection was clicked on.
        KoShape *shape = canvas()->shapeManager()->shapeAt (event->point, KoFlake::ShapeOnTop);
        if (shape) {
            shapes.append(shape);
        } else if (m_guideLine->isSelected()) {
            GuidesTool *guidesTool = dynamic_cast<GuidesTool*>(KoToolManager::instance()->toolById(canvas(), GuidesToolId));
            if (guidesTool) {
                guidesTool->editGuideLine(m_guideLine->orientation(), m_guideLine->index());
                activateTool(guidesTool->toolId());
                return;
            }
        }
    }

    QList<KoShape*> shapes2;
    foreach (KoShape *shape, shapes) {
        QSet<KoShape*> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) {
            shapes2.append(shape);
        } else {
            foreach (KoShape *delegatedShape, delegates) {
                shapes2.append(delegatedShape);
            }
        }
    }


    KoToolManager::instance()->switchToolRequested(
            KoToolManager::instance()->preferredToolForSelection(shapes2));
}

bool DefaultTool::moveSelection(int direction, Qt::KeyboardModifiers modifiers)
{
    qreal x=0.0, y=0.0;
    if (direction == Qt::Key_Left)
        x = -5;
    else if (direction == Qt::Key_Right)
        x = 5;
    else if (direction == Qt::Key_Up)
        y = -5;
    else if (direction == Qt::Key_Down)
        y = 5;

    if (x != 0.0 || y != 0.0) { // actually move
        if ((modifiers & Qt::ShiftModifier) != 0) {
            x *= 10;
            y *= 10;
        } else if ((modifiers & Qt::AltModifier) != 0) { // more precise
            x /= 5;
            y /= 5;
        }

        QList<QPointF> prevPos;
        QList<QPointF> newPos;
        QList<KoShape*> shapes;
        foreach(KoShape* shape, koSelection()->selectedShapes(KoFlake::TopLevelSelection)) {
            if (shape->isGeometryProtected())
                continue;
            shapes.append(shape);
            QPointF p = shape->position();
            prevPos.append(p);
            p.setX(p.x() + x);
            p.setY(p.y() + y);
            newPos.append(p);
        }
        if (shapes.count() > 0) {
            // use a timeout to make sure we don't reuse a command possibly deleted by the commandHistory
            if (m_lastUsedMoveCommand.msecsTo(QTime::currentTime()) > 5000)
                m_moveCommand = 0;
            if (m_moveCommand) { // alter previous instead of creating new one.
                m_moveCommand->setNewPositions(newPos);
                m_moveCommand->redo();
            } else {
                m_moveCommand = new KoShapeMoveCommand(shapes, prevPos, newPos);
                canvas()->addCommand(m_moveCommand);
            }
            m_lastUsedMoveCommand = QTime::currentTime();
            return true;
        }
    }
    return false;
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
            if (moveSelection(event->key(), event->modifiers()))
                event->accept();
            break;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
            canvas()->resourceManager()->setResource(HotPosition, event->key()-Qt::Key_1);
            event->accept();
            break;
        default:
            return;
        }
    }
}

void DefaultTool::customMoveEvent(KoPointerEvent * event)
{
    if (! koSelection()->count()) {
        event->ignore();
        return;
    }

    int move = qMax(qAbs(event->x()), qAbs(event->y()));
    int zoom = qAbs(event->z());
    int rotate = qAbs(event->rotationZ());
    const int threshold = 2;

    if (move < threshold && zoom < threshold && rotate < threshold) {
        if (m_customEventStrategy) {
            m_customEventStrategy->finishInteraction(event->modifiers());
            QUndoCommand *command = m_customEventStrategy->createCommand();
            if (command)
                canvas()->addCommand(command);
            delete m_customEventStrategy;
            m_customEventStrategy = 0;
            repaintDecorations();
        }
        event->accept();
        return;
    }

    // check if the z-movement is dominant
    if (zoom > move && zoom > rotate) {
        // zoom
        if (! m_customEventStrategy)
            m_customEventStrategy = new ShapeResizeStrategy(this, event->point, KoFlake::TopLeftHandle);
    } else if (move > zoom && move > rotate) { // check if x-/y-movement is dominant
        // move
        if (! m_customEventStrategy)
            m_customEventStrategy = new ShapeMoveStrategy(this, event->point);
    } else if (rotate > zoom && rotate > move) // rotation is dominant
    {
        // rotate
        if (! m_customEventStrategy)
            m_customEventStrategy = new ShapeRotateStrategy(this, event->point, event->buttons());
    }

    if (m_customEventStrategy)
        m_customEventStrategy->handleCustomEvent(event);

    event->accept();
}

void DefaultTool::repaintDecorations()
{
    Q_ASSERT(koSelection());
    if (koSelection()->count() > 0)
        canvas()->updateCanvas(handlesSize());
}

void DefaultTool::copy() const
{
    QList<KoShape *> shapes = canvas()->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection);
    if (!shapes.empty()) {
        KoShapeOdfSaveHelper saveHelper(shapes);
        KoDrag drag;
        drag.setOdf(KoOdf::mimeType(KoOdf::Text), saveHelper);
        drag.addToClipboard();
    }
}

void DefaultTool::deleteSelection()
{
    QList<KoShape *> shapes;
    foreach (KoShape *s, canvas()->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection)) {
        if (s->isGeometryProtected())
            continue;
        shapes << s;
    }
    if (!shapes.empty()) {
        canvas()->addCommand(canvas()->shapeController()->removeShapes(shapes));
    }
}

bool DefaultTool::paste()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    bool success = false;
    if (data->hasFormat(KoOdf::mimeType(KoOdf::Text))) {
        KoShapeManager * shapeManager = canvas()->shapeManager();
        KoShapePaste paste(canvas(), shapeManager->selection()->activeLayer());
        success = paste.paste(KoOdf::Text, data);
        if (success) {
            shapeManager->selection()->deselectAll();
            foreach(KoShape *shape, paste.pastedShapes()) {
                shapeManager->selection()->select(shape);
            }
        }
    }
    return success;
}

QStringList DefaultTool::supportedPasteMimeTypes() const
{
    QStringList list;
    list << KoOdf::mimeType(KoOdf::Text);
    return list;
}

KoSelection *DefaultTool::koSelection()
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->shapeManager());
    return canvas()->shapeManager()->selection();
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

    if (koSelection()->count() == 0)
        return KoFlake::NoHandle;

    recalcSelectionBox();
    const KoViewConverter *converter = canvas()->viewConverter();
    if (!converter) return KoFlake::NoHandle;

    if (innerHandleMeaning != 0)
    {
        QPainterPath path;
        path.addPolygon(m_selectionOutline);
        *innerHandleMeaning = path.contains(point) || path.intersects(handlePaintRect(point));
    }
    for (int i = 0; i < KoFlake::NoHandle; ++i) {
        KoFlake::SelectionHandle handle = handleOrder[i];
        QPointF pt = converter->documentToView(point) - converter->documentToView(m_selectionBox[handle]);

        // if just inside the outline
        if (qAbs(pt.x()) < HANDLE_DISTANCE &&
                qAbs(pt.y()) < HANDLE_DISTANCE) {
            if (innerHandleMeaning != 0)
            {
                if (qAbs(pt.x()) < 4 && qAbs(pt.y()) < 4)
                    *innerHandleMeaning = true;
            }
            return handle;
        }
    }
    return KoFlake::NoHandle;
}

void DefaultTool::recalcSelectionBox()
{
    if (koSelection()->count()==0)
        return;

    if (koSelection()->count()>1) {
        QTransform matrix = koSelection()->absoluteTransformation(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->size())));
        m_angle = 0.0; //koSelection()->rotation();
    } else {
        QTransform matrix = koSelection()->firstSelectedShape()->absoluteTransformation(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->firstSelectedShape()->size())));
        m_angle = 0.0; //koSelection()->firstSelectedShape()->rotation();
    }
    QPolygonF outline = m_selectionOutline; //shorter name in the following :)
    m_selectionBox[KoFlake::TopMiddleHandle] = (outline.value(0)+outline.value(1))/2;
    m_selectionBox[KoFlake::TopRightHandle] = outline.value(1);
    m_selectionBox[KoFlake::RightMiddleHandle] = (outline.value(1)+outline.value(2))/2;
    m_selectionBox[KoFlake::BottomRightHandle] = outline.value(2);
    m_selectionBox[KoFlake::BottomMiddleHandle] = (outline.value(2)+outline.value(3))/2;
    m_selectionBox[KoFlake::BottomLeftHandle] = outline.value(3);
    m_selectionBox[KoFlake::LeftMiddleHandle] = (outline.value(3)+outline.value(0))/2;
    m_selectionBox[KoFlake::TopLeftHandle] = outline.value(0);
    if (koSelection()->count() == 1) {
#if 0        // TODO detect mirroring
        KoShape *s = koSelection()->firstSelectedShape();

        if (s->scaleX() < 0) { // vertically mirrored: swap left / right
            qSwap(m_selectionBox[KoFlake::TopLeftHandle], m_selectionBox[KoFlake::TopRightHandle]);
            qSwap(m_selectionBox[KoFlake::LeftMiddleHandle], m_selectionBox[KoFlake::RightMiddleHandle]);
            qSwap(m_selectionBox[KoFlake::BottomLeftHandle], m_selectionBox[KoFlake::BottomRightHandle]);
        }
        if (s->scaleY() < 0) { // vertically mirrored: swap top / bottom
            qSwap(m_selectionBox[KoFlake::TopLeftHandle], m_selectionBox[KoFlake::BottomLeftHandle]);
            qSwap(m_selectionBox[KoFlake::TopMiddleHandle], m_selectionBox[KoFlake::BottomMiddleHandle]);
            qSwap(m_selectionBox[KoFlake::TopRightHandle], m_selectionBox[KoFlake::BottomRightHandle]);
        }
#endif
    }
}

void DefaultTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    m_mouseWasInsideHandles = false;
    m_lastHandle = KoFlake::NoHandle;
    useCursor(Qt::ArrowCursor);
    repaintDecorations();
    delete m_guideLine;
    m_guideLine = new GuideLine();
    updateActions();
}

void DefaultTool::selectionAlignHorizontalLeft()
{
    selectionAlign(KoShapeAlignCommand::HorizontalLeftAlignment);
}

void DefaultTool::selectionAlignHorizontalCenter()
{
    selectionAlign(KoShapeAlignCommand::HorizontalCenterAlignment);
}

void DefaultTool::selectionAlignHorizontalRight()
{
    selectionAlign(KoShapeAlignCommand::HorizontalRightAlignment);
}

void DefaultTool::selectionAlignVerticalTop()
{
    selectionAlign(KoShapeAlignCommand::VerticalTopAlignment);
}

void DefaultTool::selectionAlignVerticalCenter()
{
    selectionAlign(KoShapeAlignCommand::VerticalCenterAlignment);
}

void DefaultTool::selectionAlignVerticalBottom()
{
    selectionAlign(KoShapeAlignCommand::VerticalBottomAlignment);
}

void DefaultTool::selectionGroup()
{
    KoSelection* selection = koSelection();
    if (! selection)
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes(KoFlake::TopLevelSelection);
    QList<KoShape*> groupedShapes;

    // only group shapes with an unselected parent
    foreach (KoShape* shape, selectedShapes) {
        if (! selectedShapes.contains(shape->parent()) && shape->isEditable()) {
            groupedShapes << shape;
        }
    }
    KoShapeGroup *group = new KoShapeGroup();
    // TODO what if only one shape is left?
    QUndoCommand *cmd = new QUndoCommand(i18n("Group shapes"));
    canvas()->shapeController()->addShapeDirect(group, cmd);
    KoShapeGroupCommand::createCommand(group, groupedShapes, cmd);
    canvas()->addCommand(cmd);
}

void DefaultTool::selectionUngroup()
{
    KoSelection* selection = canvas()->shapeManager()->selection();
    if (! selection)
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes(KoFlake::TopLevelSelection);
    QList<KoShape*> containerSet;

    // only ungroup shape groups with an unselected parent
    foreach (KoShape* shape, selectedShapes) {
        if (!selectedShapes.contains(shape->parent()) && shape->isEditable()) {
            containerSet << shape;
        }
    }

    QUndoCommand *cmd = 0;

    // add a ungroup command for each found shape container to the macro command
    foreach(KoShape* shape, containerSet) {
        KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape);
        if (group) {
            cmd = cmd ? cmd : new QUndoCommand(i18n("Ungroup shapes"));
            new KoShapeUngroupCommand(group, group->shapes(),
                                      group->parent()? QList<KoShape*>(): canvas()->shapeManager()->topLevelShapes(),
                                      cmd);
            canvas()->shapeController()->removeShape(group, cmd);
        }
    }
    if (cmd) {
        canvas()->addCommand(cmd);
    }
}

void DefaultTool::selectionAlign(KoShapeAlignCommand::Align align)
{
    KoSelection* selection = canvas()->shapeManager()->selection();
    if (! selection)
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes(KoFlake::TopLevelSelection);
    if (selectedShapes.count() < 1)
        return;

    QList<KoShape*> editableShapes = filterEditableShapes(selectedShapes);

    // TODO add an option to the widget so that one can align to the page
    // with multiple selected shapes too

    QRectF bb;

    // single selected shape is automatically aligned to document rect
    if (editableShapes.count() == 1 ) {
        if (!canvas()->resourceManager()->hasResource(KoCanvasResource::PageSize))
            return;
        bb = QRectF(QPointF(0,0), canvas()->resourceManager()->sizeResource(KoCanvasResource::PageSize));
    } else {
        foreach( KoShape * shape, editableShapes ) {
            bb |= shape->boundingRect();
        }
    }

    KoShapeAlignCommand *cmd = new KoShapeAlignCommand(editableShapes, align, bb);

    canvas()->addCommand(cmd);
    selection->updateSizeAndPosition();
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
    KoSelection* selection = canvas()->shapeManager()->selection();
    if (! selection)
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes(KoFlake::TopLevelSelection);
    if (selectedShapes.count() < 1)
        return;

    QList<KoShape*> editableShapes = filterEditableShapes(selectedShapes);
    if (editableShapes.count() < 1)
        return;

    QUndoCommand * cmd = KoShapeReorderCommand::createCommand(editableShapes, canvas()->shapeManager(), order);
    if (cmd) {
        canvas()->addCommand(cmd);
    }
}

QMap<QString, QWidget *> DefaultTool::createOptionWidgets()
{
    QMap<QString, QWidget *> widgets;
    widgets.insert(i18n("Arrange"), new DefaultToolArrangeWidget(this));
    widgets.insert(i18n("Geometry"), new DefaultToolWidget(this));
    widgets.insert(i18n("Snapping"), canvas()->createSnapGuideConfigWidget());
    return widgets;
}

void DefaultTool::resourceChanged(int key, const QVariant & res)
{
    if (key == HotPosition) {
        m_hotPosition = static_cast<KoFlake::Position>(res.toInt());
        repaintDecorations();
    }
}

KoInteractionStrategy *DefaultTool::createStrategy(KoPointerEvent *event)
{
    // reset the move by keys when a new strategy is created otherwise we might change the
    // command after a new command was added. This happend when you where faster than the timer.
    m_moveCommand = 0;

    KoShapeManager *shapeManager = canvas()->shapeManager();
    KoSelection *select = shapeManager->selection();
    bool insideSelection;
    KoFlake::SelectionHandle handle = handleAt(event->point, &insideSelection);

    bool editableShape = editableShapesCount(select->selectedShapes());

    if (event->buttons() & Qt::MidButton) {
        // change the hot selection position when middle clicking on a handle
        KoFlake::Position newHotPosition = m_hotPosition;
        switch (handle) {
        case KoFlake::TopLeftHandle:
            newHotPosition = KoFlake::TopLeftCorner;
            break;
        case KoFlake::TopRightHandle:
            newHotPosition = KoFlake::TopRightCorner;
            break;
        case KoFlake::BottomLeftHandle:
            newHotPosition = KoFlake::BottomLeftCorner;
            break;
        case KoFlake::BottomRightHandle:
            newHotPosition = KoFlake::BottomRightCorner;
            break;
        default:
        {
            // check if we had hit the center point
            const KoViewConverter * converter = canvas()->viewConverter();
            QPointF pt = converter->documentToView(event->point-select->absolutePosition());
            if (qAbs(pt.x()) < HANDLE_DISTANCE && qAbs(pt.y()) < HANDLE_DISTANCE)
                newHotPosition = KoFlake::CenteredPosition;
            break;
        }
        }
        if (m_hotPosition != newHotPosition)
            canvas()->resourceManager()->setResource(HotPosition, newHotPosition);
        return 0;
    }

    bool selectMultiple = event->modifiers() & Qt::ControlModifier;
    bool selectNextInStack = event->modifiers() & Qt::ShiftModifier;

    if (editableShape) {
        // manipulation of selected shapes goes first
        if (handle != KoFlake::NoHandle) {
            if (event->buttons() == Qt::LeftButton) {
                // resizing or shearing only with left mouse button
                if (insideSelection)
                    return new ShapeResizeStrategy(this, event->point, handle);
                if (handle == KoFlake::TopMiddleHandle || handle == KoFlake::RightMiddleHandle ||
                        handle == KoFlake::BottomMiddleHandle || handle == KoFlake::LeftMiddleHandle)
                    return new ShapeShearStrategy(this, event->point, handle);
            }
            // rotating is allowed for rigth mouse button too
            if (handle == KoFlake::TopLeftHandle || handle == KoFlake::TopRightHandle ||
                handle == KoFlake::BottomLeftHandle || handle == KoFlake::BottomRightHandle)
                return new ShapeRotateStrategy(this, event->point, event->buttons());
        }
        if (! (selectMultiple || selectNextInStack) && event->buttons() == Qt::LeftButton) {
            const QPainterPath outlinePath = select->transformation().map(select->outline());
            if (outlinePath.contains(event->point) || outlinePath.intersects(handlePaintRect(event->point)))
                return new ShapeMoveStrategy(this, event->point);
        }
    }

    if ((event->buttons() & Qt::LeftButton) == 0)
        return 0;  // Nothing to do for middle/right mouse button

    KoShape *shape = shapeManager->shapeAt(event->point, selectNextInStack ? KoFlake::NextUnselected : KoFlake::ShapeOnTop);

    if (!shape && handle == KoFlake::NoHandle) {
        // check if we have hit a guide
        if (m_guideLine->isValid()) {
            m_guideLine->select();
            return 0;
        }
        if (! selectMultiple) {
            repaintDecorations();
            select->deselectAll();
        }
        return new KoShapeRubberSelectStrategy(this, event->point);
    }

    if (select->isSelected(shape)) {
        if (selectMultiple) {
            repaintDecorations();
            select->deselect(shape);
        }
    }
    else if (handle == KoFlake::NoHandle) { // clicked on shape which is not selected
        repaintDecorations();
        if (! selectMultiple)
            shapeManager->selection()->deselectAll();
        select->select(shape, selectNextInStack ? false : true);
        repaintDecorations();
        return new ShapeMoveStrategy(this, event->point);
    }
    return 0;
}

void DefaultTool::updateActions()
{
    KoSelection * selection(koSelection());
    if (!selection) {
        action("object_order_front")->setEnabled(false);
        action("object_order_raise")->setEnabled(false);
        action("object_order_lower")->setEnabled(false);
        action("object_order_back")->setEnabled(false);
        action("object_align_horizontal_left")->setEnabled(false);
        action("object_align_horizontal_center")->setEnabled(false);
        action("object_align_horizontal_right")->setEnabled(false);
        action("object_align_vertical_top")->setEnabled(false);
        action("object_align_vertical_center")->setEnabled(false);
        action("object_align_vertical_bottom")->setEnabled(false);
        action("object_group")->setEnabled(false);
        action("object_ungroup")->setEnabled(false);
        return;
    }

    QList<KoShape*> editableShapes = filterEditableShapes(selection->selectedShapes(KoFlake::TopLevelSelection));
    bool enable = editableShapes.count () > 0;
    action("object_order_front")->setEnabled(enable);
    action("object_order_raise")->setEnabled(enable);
    action("object_order_lower")->setEnabled(enable);
    action("object_order_back")->setEnabled(enable);
    enable = (editableShapes.count () > 1) || (enable && canvas()->resourceManager()->hasResource(KoCanvasResource::PageSize));
    action("object_align_horizontal_left")->setEnabled(enable);
    action("object_align_horizontal_center")->setEnabled(enable);
    action("object_align_horizontal_right")->setEnabled(enable);
    action("object_align_vertical_top")->setEnabled(enable);
    action("object_align_vertical_center")->setEnabled(enable);
    action("object_align_vertical_bottom")->setEnabled(enable);

    action("object_group")->setEnabled(editableShapes.count() > 1);
    bool groupShape = false;
    foreach (KoShape * shape, editableShapes) {
        if (dynamic_cast<KoShapeGroup *>(shape)) {
            groupShape = true;
            break;
        }
    }
    action("object_ungroup")->setEnabled(groupShape);

    emit selectionChanged(selection->count());
}

KoToolSelection* DefaultTool::selection()
{
    return m_selectionHandler;
}

QList<KoShape*> DefaultTool::filterEditableShapes( const QList<KoShape*> &shapes )
{
    QList<KoShape*> editableShapes;
    foreach( KoShape * shape, shapes ) {
        if (shape->isEditable())
            editableShapes.append(shape);
    }

    return editableShapes;
}

uint DefaultTool::editableShapesCount( const QList<KoShape*> &shapes )
{
    uint count = 0;
    foreach( KoShape * shape, shapes ) {
        if (shape->isEditable())
            count++;
    }

    return count;
}

#include <DefaultTool.moc>
