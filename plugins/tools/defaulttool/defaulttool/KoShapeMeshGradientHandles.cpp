/*
 *  SPDX-FileCopyrightText: 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoShapeMeshGradientHandles.h"

#include <QVector>

#include <KoShapeFillWrapper.h>
#include <kis_algebra_2d.h>

KoShapeMeshGradientHandles::KoShapeMeshGradientHandles(KoFlake::FillVariant fillVariant,
                                                       KoShape *shape)
    : m_fillVariant(fillVariant)
    , m_shape(shape)
{
}

QVector<KoShapeMeshGradientHandles::Handle> KoShapeMeshGradientHandles::handles() const
{
    QVector<Handle> result;

    const SvgMeshGradient *g = gradient();
    if (!g) return result;

    SvgMeshArray const *mesharray = g->getMeshArray().data();

    for (int irow = 0; irow < mesharray->numRows(); ++irow) {
        for (int icol = 0; icol < mesharray->numColumns(); ++icol) {
            // add corners as well
            result << getHandles(mesharray, SvgMeshPatch::Top, irow, icol);

            result << getBezierHandles(mesharray, SvgMeshPatch::Left, irow, icol);

            if (irow == mesharray->numRows() - 1) {
                result << getHandles(mesharray, SvgMeshPatch::Left, irow, icol);

                if (icol == mesharray->numColumns() - 1) {
                    result << getHandles(mesharray, SvgMeshPatch::Bottom, irow, icol);
                } else {
                    result << getBezierHandles(mesharray, SvgMeshPatch::Bottom, irow, icol);
                }
            }

            if (icol == mesharray->numColumns() - 1) {
                result << getHandles(mesharray, SvgMeshPatch::Right, irow, icol);
            }
        }
    }

    // we get pointer events in points (pts, not logical), so we transform these now
    // and then invert them while drawing handles (see SelectionDecorator).
    QTransform t = abosoluteTransformation(g->gradientUnits());
    for (auto &handle: result) {
        handle.pos = t.map(handle.pos);
    }

    return result;
}

KoShapeMeshGradientHandles::Handle KoShapeMeshGradientHandles::getHandle(SvgMeshPosition position) const
{
    const SvgMeshGradient *g = gradient();
    if (!g) return Handle();

    Handle handle = getHandles(g->getMeshArray().data(), position.segmentType, position.row, position.col)[0];

    QTransform t = abosoluteTransformation(g->gradientUnits());
    handle.pos = t.map(handle.pos);

    return handle;
}

KUndo2Command* KoShapeMeshGradientHandles::moveGradientHandle(const Handle &handle,
                                                              const QPointF &newPos)
{
    KoShapeFillWrapper wrapper(m_shape, m_fillVariant);
    QScopedPointer<SvgMeshGradient> newGradient(new SvgMeshGradient(*wrapper.meshgradient()));
    SvgMeshArray *mesharray = newGradient->getMeshArray().data();
    SvgMeshPatch *patch = newGradient->getMeshArray()->getPatch(handle.row, handle.col);
    std::array<QPointF, 4> path = patch->getSegment(handle.segmentType);

    QTransform t = abosoluteTransformation(newGradient->gradientUnits()).inverted();

    if (handle.type == Handle::BezierHandle) {
        path[handle.index] = t.map(newPos);
        mesharray->modifyHandle(SvgMeshPosition {handle.row, handle.col, handle.segmentType}, path);

    } else if (handle.type == Handle::Corner) {
        mesharray->modifyCorner(SvgMeshPosition {handle.row, handle.col, handle.segmentType}, t.map(newPos));
    }

    return wrapper.setMeshGradient(newGradient.data(), QTransform());
}

QPainterPath KoShapeMeshGradientHandles::path() const
{
    QPainterPath painterPath;

    if (!gradient())
        return painterPath;

    QScopedPointer<SvgMeshGradient> g(new SvgMeshGradient(*gradient()));
    if (g->gradientUnits() == KoFlake::ObjectBoundingBox) {
        const QTransform gradientToUser = KisAlgebra2D::mapToRect(m_shape->outlineRect());
        g->setTransform(gradientToUser);
    }

    SvgMeshArray *mesharray = g->getMeshArray().data();

    for (int i = 0; i < mesharray->numRows(); ++i) {
        for (int j = 0; j < mesharray->numColumns(); ++j) {
            painterPath.addPath(mesharray->getPatch(i, j)->getPath());
        }
    }

    return painterPath;
}

QVector<QPainterPath> KoShapeMeshGradientHandles::getConnectedPath(const Handle &handle) const
{
    KIS_ASSERT(handle.type != Handle::None);

    QVector<QPainterPath> result;

    const SvgMeshArray *mesharray = gradient()->getMeshArray().data();
    QPainterPath painterPath;

    if (handle.type == Handle::BezierHandle) {
        SvgMeshPath path = mesharray->getPath(handle.getPosition());
        painterPath.moveTo(path[0]);
        painterPath.cubicTo(path[1], path[2], path[3]);
        result << painterPath;
    } else {
        QVector<SvgMeshPosition> positions = mesharray->getConnectedPaths(handle.getPosition());
        for (const auto &position: positions) {
            SvgMeshPath path = mesharray->getPath(position);
            painterPath = QPainterPath();
            painterPath.moveTo(path[0]);
            painterPath.cubicTo(path[1], path[2], path[3]);
            result << painterPath;
        }
    }

    return result;
}

QPointF KoShapeMeshGradientHandles::getAttachedCorner(const Handle &bezierHandle) const
{
    KIS_ASSERT(bezierHandle.type == Handle::BezierHandle);

    const SvgMeshArray *mesharray = gradient()->getMeshArray().data();
    const SvgMeshPath path = mesharray->getPath(bezierHandle.getPosition());
    const QTransform t = abosoluteTransformation(gradient()->gradientUnits());
    if (bezierHandle.index == Handle::First) {
        return t.map(path[bezierHandle.index - 1]);
    } else {
        return t.map(path[bezierHandle.index + 1]);
    }
}

const SvgMeshGradient* KoShapeMeshGradientHandles::gradient() const
{
    KoShapeFillWrapper wrapper(m_shape, m_fillVariant);
    return wrapper.meshgradient();
}

QVector<KoShapeMeshGradientHandles::Handle> KoShapeMeshGradientHandles::getHandles(const SvgMeshArray *mesharray,
                                                                                   SvgMeshPatch::Type type,
                                                                                   int row,
                                                                                   int col) const
{
    QVector<Handle> buffer;
    std::array<QPointF, 4> path = mesharray->getPath(type, row, col);
    buffer << Handle(Handle::Corner, path[0], row, col, type);
    buffer << Handle(Handle::BezierHandle, path[1], row, col, type, Handle::First);
    buffer << Handle(Handle::BezierHandle, path[2], row, col, type, Handle::Second);

    return buffer;
}

QVector<KoShapeMeshGradientHandles::Handle> KoShapeMeshGradientHandles::getBezierHandles(const SvgMeshArray *mesharray,
                                                                                         SvgMeshPatch::Type type,
                                                                                         int row,
                                                                                         int col) const
{
    QVector<Handle> buffer;
    std::array<QPointF, 4> path = mesharray->getPath(type, row, col);
    buffer << Handle(Handle::BezierHandle, path[1], row, col, type, Handle::First);
    buffer << Handle(Handle::BezierHandle, path[2], row, col, type, Handle::Second);

    return buffer;
}

QTransform KoShapeMeshGradientHandles::abosoluteTransformation(KoFlake::CoordinateSystem system) const
{
    QTransform t;
    if (system == KoFlake::UserSpaceOnUse) {
        t = m_shape->absoluteTransformation();
    } else {
        const QTransform gradientToUser = KisAlgebra2D::mapToRect(m_shape->outlineRect());
        t = gradientToUser * m_shape->absoluteTransformation();
    }
    return t;
}
