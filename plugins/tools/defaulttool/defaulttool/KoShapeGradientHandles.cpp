/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoShapeGradientHandles.h"

#include <QGradient>
#include <KoShape.h>
#include <KoGradientBackground.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeFillWrapper.h>
#include <kis_assert.h>
#include "kis_algebra_2d.h"

KoShapeGradientHandles::KoShapeGradientHandles(KoFlake::FillVariant fillVariant, KoShape *shape)
    : m_fillVariant(fillVariant),
      m_shape(shape)
{
}

QVector<KoShapeGradientHandles::Handle> KoShapeGradientHandles::handles() const {
    QVector<Handle> result;

    const QGradient *g = gradient();
    if (!g) return result;

    switch (g->type()) {
    case QGradient::LinearGradient: {
        const QLinearGradient *lgradient = static_cast<const QLinearGradient*>(g);
        result << Handle(Handle::LinearStart, lgradient->start());
        result << Handle(Handle::LinearEnd, lgradient->finalStop());
        break;
    }
    case QGradient::RadialGradient: {
        const QRadialGradient *rgradient = static_cast<const QRadialGradient*>(g);

        result << Handle(Handle::RadialCenter, rgradient->center());

        if (rgradient->center() != rgradient->focalPoint()) {
            result << Handle(Handle::RadialFocalPoint, rgradient->focalPoint());
        }

        result << Handle(Handle::RadialRadius,
                         rgradient->center() + QPointF(rgradient->centerRadius(), 0));
        break;
    }
    case QGradient::ConicalGradient:
        // not supported
        break;
    case QGradient::NoGradient:
        // not supported
        break;
    }

    if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
        const QRectF boundingRect = m_shape->outlineRect();
        const QTransform gradientToUser(boundingRect.width(), 0, 0, boundingRect.height(),
                                        boundingRect.x(), boundingRect.y());
        const QTransform t = gradientToUser * m_shape->absoluteTransformation();

        QVector<Handle>::iterator it = result.begin();



        for (; it != result.end(); ++it) {
            it->pos = t.map(it->pos);
        }
    }

    return result;
}

QGradient::Type KoShapeGradientHandles::type() const
{
    const QGradient *g = gradient();
    return g ? g->type() : QGradient::NoGradient;
}

KUndo2Command *KoShapeGradientHandles::moveGradientHandle(KoShapeGradientHandles::Handle::Type handleType, const QPointF &absoluteOffset)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(handleType != Handle::None, 0);

    KoShapeFillWrapper wrapper(m_shape, m_fillVariant);
    const QGradient *originalGradient = wrapper.gradient();
    QTransform originalTransform = wrapper.gradientTransform();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(originalGradient, 0);

    QScopedPointer<QGradient> newGradient;

    switch (originalGradient->type()) {
    case QGradient::LinearGradient: {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(handleType == Handle::LinearStart ||
                                             handleType == Handle::LinearEnd, 0);

        newGradient.reset(KoFlake::cloneGradient(originalGradient));
        QLinearGradient *lgradient = static_cast<QLinearGradient*>(newGradient.data());

        if (handleType == Handle::LinearStart) {
            lgradient->setStart(getNewHandlePos(lgradient->start(), absoluteOffset, newGradient->coordinateMode()));
        } else if (handleType == Handle::LinearEnd) {
            lgradient->setFinalStop(getNewHandlePos(lgradient->finalStop(), absoluteOffset, newGradient->coordinateMode()));

        }
        break;
    }
    case QGradient::RadialGradient: {
        newGradient.reset(KoFlake::cloneGradient(originalGradient));
        QRadialGradient *rgradient = static_cast<QRadialGradient*>(newGradient.data());

        if (handleType == Handle::RadialCenter) {
            rgradient->setCenter(getNewHandlePos(rgradient->center(), absoluteOffset, newGradient->coordinateMode()));
        } else if (handleType == Handle::RadialFocalPoint) {
            rgradient->setFocalPoint(getNewHandlePos(rgradient->focalPoint(), absoluteOffset, newGradient->coordinateMode()));
        } else if (handleType == Handle::RadialRadius) {
            QPointF radiusPos = rgradient->center() + QPointF(QPointF(rgradient->radius(), 0));
            radiusPos = getNewHandlePos(radiusPos, absoluteOffset, newGradient->coordinateMode());
            rgradient->setRadius(radiusPos.x() - rgradient->center().x());
        }
        break;
    }
    case QGradient::ConicalGradient:
        // not supported
        break;
    case QGradient::NoGradient:
        // not supported
        break;
    }

    return wrapper.setGradient(newGradient.data(), originalTransform);
}

KoShapeGradientHandles::Handle KoShapeGradientHandles::getHandle(KoShapeGradientHandles::Handle::Type handleType)
{
    Handle result;

    Q_FOREACH (const Handle &h, handles()) {
        if (h.type == handleType) {
            result = h;
            break;
        }
    }

    return result;
}

const QGradient *KoShapeGradientHandles::gradient() const {
    KoShapeFillWrapper wrapper(m_shape, m_fillVariant);
    return wrapper.gradient();
}

QPointF KoShapeGradientHandles::getNewHandlePos(const QPointF &oldPos, const QPointF &absoluteOffset, QGradient::CoordinateMode mode)
{
    const QTransform offset = QTransform::fromTranslate(absoluteOffset.x(), absoluteOffset.y());
    QTransform localToAbsolute = m_shape->absoluteTransformation();
    QTransform absoluteToLocal = localToAbsolute.inverted();

    if (mode == QGradient::ObjectBoundingMode) {
        const QRectF rect = m_shape->outlineRect();
        const QTransform gradientToUser = KisAlgebra2D::mapToRect(rect);
        localToAbsolute = gradientToUser * localToAbsolute;

        /// Some shapes may have zero-width/height, then inverted transform will not
        /// exist. Therefore we should use a special method for that.
        const QTransform userToGradient = KisAlgebra2D::mapToRectInverse(rect);
        absoluteToLocal = absoluteToLocal * userToGradient;
    }

    return (localToAbsolute * offset * absoluteToLocal).map(oldPos);
}
