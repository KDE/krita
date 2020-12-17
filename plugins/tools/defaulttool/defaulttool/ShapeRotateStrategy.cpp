/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007-2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ShapeRotateStrategy.h"
#include "SelectionDecorator.h"

#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoCanvasResourceProvider.h>
#include <commands/KoShapeTransformCommand.h>

#include <QPointF>
#include <math.h>
#include <klocalizedstring.h>

ShapeRotateStrategy::ShapeRotateStrategy(KoToolBase *tool, KoSelection *selection, const QPointF &clicked, Qt::MouseButtons buttons)
    : KoInteractionStrategy(tool)
    , m_start(clicked)
{
    /**
     * The outline of the selection should look as if it is also rotated, so we
     * add it to the transformed shapes list.
     */
    m_transformedShapesAndSelection = selection->selectedEditableShapes();
    m_transformedShapesAndSelection << selection;

    Q_FOREACH (KoShape *shape, m_transformedShapesAndSelection) {
        m_oldTransforms << shape->transformation();
    }

    KoFlake::AnchorPosition anchor = !(buttons & Qt::RightButton) ?
                KoFlake::Center :
                KoFlake::AnchorPosition(tool->canvas()->resourceManager()->resource(KoFlake::HotPosition).toInt());

    m_rotationCenter = selection->absolutePosition(anchor);

    tool->setStatusText(i18n("Press ALT to rotate in 45 degree steps."));
}

void ShapeRotateStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    qreal angle = atan2(point.y() - m_rotationCenter.y(), point.x() - m_rotationCenter.x()) -
                  atan2(m_start.y() - m_rotationCenter.y(), m_start.x() - m_rotationCenter.x());
    angle = angle / M_PI * 180;  // convert to degrees.
    if (modifiers & (Qt::AltModifier | Qt::ControlModifier)) {
        // limit to 45 degree angles
        qreal modula = qAbs(angle);
        while (modula > 45.0) {
            modula -= 45.0;
        }
        if (modula > 22.5) {
            modula -= 45.0;
        }
        angle += (angle > 0 ? -1 : 1) * modula;
    }

    rotateBy(angle);
}

void ShapeRotateStrategy::rotateBy(qreal angle)
{
    QTransform matrix;
    matrix.translate(m_rotationCenter.x(), m_rotationCenter.y());
    matrix.rotate(angle);
    matrix.translate(-m_rotationCenter.x(), -m_rotationCenter.y());

    QTransform applyMatrix = matrix * m_rotationMatrix.inverted();
    m_rotationMatrix = matrix;
    Q_FOREACH (KoShape *shape, m_transformedShapesAndSelection) {
        QRectF dirtyRect = shape->boundingRect();
        shape->applyAbsoluteTransformation(applyMatrix);
        dirtyRect |= shape->boundingRect();
        shape->updateAbsolute(dirtyRect);
    }
}

void ShapeRotateStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    // paint the rotation center
    painter.setPen(QPen(Qt::red));
    painter.setBrush(QBrush(Qt::red));
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRectF circle(0, 0, 5, 5);
    circle.moveCenter(converter.documentToView(m_rotationCenter));
    painter.drawEllipse(circle);
}

KUndo2Command *ShapeRotateStrategy::createCommand()
{
    QList<QTransform> newTransforms;
    Q_FOREACH (KoShape *shape, m_transformedShapesAndSelection) {
        newTransforms << shape->transformation();
    }

    KoShapeTransformCommand *cmd = new KoShapeTransformCommand(m_transformedShapesAndSelection, m_oldTransforms, newTransforms);
    cmd->setText(kundo2_i18n("Rotate"));
    return cmd;
}
