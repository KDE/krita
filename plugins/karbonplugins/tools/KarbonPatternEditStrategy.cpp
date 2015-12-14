/* This file is part of the KDE project
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonPatternEditStrategy.h"

#include <KoShape.h>
#include <KoViewConverter.h>
#include <KoShapeBackgroundCommand.h>

#include <QPainter>
#include <kundo2command.h>

#include <math.h>

uint KarbonPatternEditStrategyBase::m_handleRadius = 3;
uint KarbonPatternEditStrategyBase::m_grabSensitivity = 3;

KarbonPatternEditStrategyBase::KarbonPatternEditStrategyBase(KoShape *s, KoImageCollection *imageCollection)
    : m_selectedHandle(-1)
    , m_oldFill(new KoPatternBackground(imageCollection))
    , m_newFill(new KoPatternBackground(imageCollection))
    , m_shape(s)
    , m_imageCollection(imageCollection)
    , m_editing(false)
    , m_modified(false)
{
    Q_ASSERT(m_shape);
    Q_ASSERT(imageCollection);
    // cache the shapes transformation matrix
    m_matrix = shape()->absoluteTransformation(0);
}

KarbonPatternEditStrategyBase::~KarbonPatternEditStrategyBase()
{
}

void KarbonPatternEditStrategyBase::setEditing(bool on)
{
    m_editing = on;
    // if we are going into editing mode, save the old background
    // for use inside the command emitted when finished
    if (on) {
        m_modified = false;
        QSharedPointer<KoPatternBackground> fill = qSharedPointerDynamicCast<KoPatternBackground>(m_shape->background());
        if (fill) {
            m_oldFill = fill;
        }
    }
}

void KarbonPatternEditStrategyBase::setModified()
{
    m_modified = true;
}

bool KarbonPatternEditStrategyBase::isModified() const
{
    return m_modified;
}

KUndo2Command *KarbonPatternEditStrategyBase::createCommand()
{
    QSharedPointer<KoPatternBackground>  fill = qSharedPointerDynamicCast<KoPatternBackground>(m_shape->background());
    if (fill && isModified()) {
        fill = m_oldFill;
        QSharedPointer<KoPatternBackground> newFill(new KoPatternBackground(m_imageCollection));
        newFill = m_newFill;
        return new KoShapeBackgroundCommand(m_shape, newFill, 0);
    }
    return 0;
}

void KarbonPatternEditStrategyBase::paintHandle(QPainter &painter, const KoViewConverter &converter, const QPointF &position) const
{
    QRectF handleRect = converter.viewToDocument(QRectF(0, 0, 2 * m_handleRadius, 2 * m_handleRadius));
    handleRect.moveCenter(position);
    painter.drawRect(handleRect);
}

bool KarbonPatternEditStrategyBase::mouseInsideHandle(const QPointF &mousePos, const QPointF &handlePos, const KoViewConverter &converter) const
{
    qreal grabSensitivityInPt = converter.viewToDocumentX(m_grabSensitivity);
    if (mousePos.x() < handlePos.x() - grabSensitivityInPt) {
        return false;
    }
    if (mousePos.x() > handlePos.x() + grabSensitivityInPt) {
        return false;
    }
    if (mousePos.y() < handlePos.y() - grabSensitivityInPt) {
        return false;
    }
    if (mousePos.y() > handlePos.y() + grabSensitivityInPt) {
        return false;
    }
    return true;
}

void KarbonPatternEditStrategyBase::repaint() const
{
    m_shape->update();
}

KoShape *KarbonPatternEditStrategyBase::shape() const
{
    return m_shape;
}

KoImageCollection *KarbonPatternEditStrategyBase::imageCollection()
{
    return m_imageCollection;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

KarbonPatternEditStrategy::KarbonPatternEditStrategy(KoShape *s, KoImageCollection *imageCollection)
    : KarbonPatternEditStrategyBase(s, imageCollection)
{
    // cache the shapes transformation matrix
    m_matrix = shape()->absoluteTransformation(0);
    QSizeF size = shape()->size();
    // the fixed length of half the average shape dimension
    m_normalizedLength = 0.25 * (size.width() + size.height());
    // get the brush tranformation matrix
    QTransform brushMatrix;
    QSharedPointer<KoPatternBackground>  fill = qSharedPointerDynamicCast<KoPatternBackground>(shape()->background());
    if (fill) {
        brushMatrix = fill->transform();
    }

    // the center handle at the center point of the shape
    //m_origin = QPointF( 0.5 * size.width(), 0.5 * size.height() );
    m_handles.append(brushMatrix.map(QPointF()));
    // the direction handle with the length of half the average shape dimension
    QPointF dirVec = QPointF(m_normalizedLength, 0.0);
    m_handles.append(brushMatrix.map(dirVec));
}

KarbonPatternEditStrategy::~KarbonPatternEditStrategy()
{
}

void KarbonPatternEditStrategy::paint(QPainter &painter, const KoViewConverter &converter) const
{
    QPointF centerPoint = m_matrix.map(m_origin + m_handles[center]);
    QPointF directionPoint = m_matrix.map(m_origin + m_handles[direction]);

    KoShape::applyConversion(painter, converter);
    painter.drawLine(centerPoint, directionPoint);
    paintHandle(painter, converter, centerPoint);
    paintHandle(painter, converter, directionPoint);
}

bool KarbonPatternEditStrategy::selectHandle(const QPointF &mousePos, const KoViewConverter &converter)
{
    int handleIndex = 0;
    Q_FOREACH (const QPointF &handle, m_handles) {
        if (mouseInsideHandle(mousePos, m_matrix.map(m_origin + handle), converter)) {
            m_selectedHandle = handleIndex;
            return true;
        }
        handleIndex++;
    }
    m_selectedHandle = -1;
    return false;
}

void KarbonPatternEditStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)

    if (m_selectedHandle == direction) {
        QPointF newPos = m_matrix.inverted().map(mouseLocation) - m_origin - m_handles[center];
        // calculate the temporary length after handle movement
        qreal newLength = sqrt(newPos.x() * newPos.x() + newPos.y() * newPos.y());
        // set the new direction vector with the new direction and normalized length
        m_handles[m_selectedHandle] = m_handles[center] + m_normalizedLength / newLength * newPos;
    } else if (m_selectedHandle == center) {
        QPointF diffPos = m_matrix.inverted().map(mouseLocation) - m_origin - m_handles[center];
        m_handles[center] += diffPos;
        m_handles[direction] += diffPos;
    } else {
        return;
    }

    setModified();

    QSharedPointer<KoPatternBackground>  fill = qSharedPointerDynamicCast<KoPatternBackground>(shape()->background());
    if (fill) {
        m_newFill = updatedBackground();
        fill = m_newFill;
    }
}

QRectF KarbonPatternEditStrategy::boundingRect() const
{
    // calculate the bounding rect of the handles
    QRectF bbox(m_matrix.map(m_origin + m_handles[0]), QSize(0, 0));
    for (int i = 1; i < m_handles.count(); ++i) {
        QPointF handle = m_matrix.map(m_origin + m_handles[i]);
        bbox.setLeft(qMin(handle.x(), bbox.left()));
        bbox.setRight(qMax(handle.x(), bbox.right()));
        bbox.setTop(qMin(handle.y(), bbox.top()));
        bbox.setBottom(qMax(handle.y(), bbox.bottom()));
    }
    qreal hr = handleRadius();
    return bbox.adjusted(-hr, -hr, hr, hr);
}

QSharedPointer<KoPatternBackground> KarbonPatternEditStrategy::updatedBackground()
{
    // the direction vector controls the rotation of the pattern
    QPointF dirVec = m_handles[direction] - m_handles[center];
    qreal angle = atan2(dirVec.y(), dirVec.x()) * 180.0 / M_PI;
    QTransform matrix;
    // the center handle controls the translation
    matrix.translate(m_handles[center].x(), m_handles[center].y());
    matrix.rotate(angle);

    QSharedPointer<KoPatternBackground> newFill(new KoPatternBackground(imageCollection()));
    newFill->setTransform(matrix);

    return newFill;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

KarbonOdfPatternEditStrategy::KarbonOdfPatternEditStrategy(KoShape *s, KoImageCollection *imageCollection)
    : KarbonPatternEditStrategyBase(s, imageCollection)
{
    m_handles.append(QPointF());
    m_handles.append(QPointF());
    updateHandles(qSharedPointerDynamicCast<KoPatternBackground>(shape()->background()));
}

KarbonOdfPatternEditStrategy::~KarbonOdfPatternEditStrategy()
{
}

void KarbonOdfPatternEditStrategy::paint(QPainter &painter, const KoViewConverter &converter) const
{
    KoShape::applyConversion(painter, converter);

    QSharedPointer<KoPatternBackground>  fill = qSharedPointerDynamicCast<KoPatternBackground>(shape()->background());
    if (!fill) {
        return;
    }

    painter.save();
    painter.setTransform(m_matrix * painter.transform());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(m_handles[origin], m_handles[size]));
    painter.restore();

    if (fill->repeat() == KoPatternBackground::Tiled) {
        paintHandle(painter, converter, m_matrix.map(m_handles[origin]));
    }
    if (fill->repeat() != KoPatternBackground::Stretched) {
        paintHandle(painter, converter, m_matrix.map(m_handles[size]));
    }

}

bool KarbonOdfPatternEditStrategy::selectHandle(const QPointF &mousePos, const KoViewConverter &converter)
{
    QSharedPointer<KoPatternBackground>  fill = qSharedPointerDynamicCast<KoPatternBackground>(shape()->background());
    if (!fill) {
        return false;
    }

    if (fill->repeat() == KoPatternBackground::Stretched) {
        return false;
    }

    m_selectedHandle = -1;

    if (mouseInsideHandle(mousePos, m_matrix.map(m_handles[size]), converter)) {
        m_selectedHandle = size;
        return true;
    }

    if (fill->repeat() == KoPatternBackground::Original) {
        return false;
    }

    if (mouseInsideHandle(mousePos, m_matrix.map(m_handles[origin]), converter)) {
        m_selectedHandle = origin;
        return true;
    }

    return false;
}

void KarbonOdfPatternEditStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    QSharedPointer<KoPatternBackground> fill = qSharedPointerDynamicCast<KoPatternBackground>(shape()->background());
    if (!fill) {
        return;
    }

    if (fill->repeat() == KoPatternBackground::Stretched) {
        return;
    }

    if (m_selectedHandle == origin) {
        if (fill->repeat() == KoPatternBackground::Original) {
            return;
        }

        QPointF diffPos = m_matrix.inverted().map(mouseLocation) - m_handles[origin];
        m_handles[origin] += diffPos;
        m_handles[size] += diffPos;
    } else if (m_selectedHandle == size) {
        QPointF newPos = m_matrix.inverted().map(mouseLocation);
        newPos.setX(qMax(newPos.x(), m_handles[origin].x()));
        newPos.setY(qMax(newPos.y(), m_handles[origin].y()));
        if (fill->repeat() == KoPatternBackground::Original) {
            QPointF diffPos = newPos - m_handles[size];
            m_handles[size] += 0.5 * diffPos;
            m_handles[origin] -= 0.5 * diffPos;
        } else {
            m_handles[size] = newPos;
        }
    } else {
        return;
    }

    setModified();

    m_newFill = updatedBackground();
    updateHandles(m_newFill);
}

QRectF KarbonOdfPatternEditStrategy::boundingRect() const
{
    // calculate the bounding rect of the handles
    QRectF bbox(m_matrix.map(m_handles[origin]), m_matrix.map(m_handles[size]));
    qreal hr = handleRadius();
    return bbox.adjusted(-hr, -hr, hr, hr);
}

QSharedPointer<KoPatternBackground> KarbonOdfPatternEditStrategy::updatedBackground()
{
    QSizeF displaySize(m_handles[size].x() - m_handles[origin].x(), m_handles[size].y() - m_handles[origin].y());
    qreal offsetX = 100.0 * (m_handles[origin].x() / displaySize.width());
    qreal offsetY = 100.0 * (m_handles[origin].y() / displaySize.height());

    QSharedPointer<KoPatternBackground> newFill(new KoPatternBackground(imageCollection()));
    newFill = m_oldFill;
    newFill->setReferencePoint(KoPatternBackground::TopLeft);
    newFill->setReferencePointOffset(QPointF(offsetX, offsetY));
    newFill->setPatternDisplaySize(displaySize);

    return newFill;
}

void KarbonOdfPatternEditStrategy::updateHandles(QSharedPointer<KoPatternBackground> fill)
{
    if (!fill) {
        return;
    }

    QRectF patternRect = fill->patternRectFromFillSize(shape()->size());
    m_handles[origin] = patternRect.topLeft();
    m_handles[size] = patternRect.bottomRight();
}

void KarbonOdfPatternEditStrategy::updateHandles()
{
    updateHandles(qSharedPointerDynamicCast<KoPatternBackground>(shape()->background()));
}
