/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_transform_utils.h"

#include <cmath>
#include <QTransform>
#include "tool_transform_args.h"


const int KisTransformUtils::rotationHandleVisualRadius = 12;
const int KisTransformUtils::rotationHandleRadius = 24;
const int KisTransformUtils::handleVisualRadius = 12;
const int KisTransformUtils::handleRadius = 24;


QTransform KisTransformUtils::imageToFlakeTransform(const KisCoordinatesConverter *converter)
{
    return converter->imageToDocumentTransform() * converter->documentToFlakeTransform();
}

qreal KisTransformUtils::effectiveHandleGrabRadius(const KisCoordinatesConverter *converter)
{
    QPointF handleRadiusPt = flakeToImage(converter, QPointF(handleRadius, handleRadius));
    return (handleRadiusPt.x() > handleRadiusPt.y()) ? handleRadiusPt.x() : handleRadiusPt.y();
}

qreal KisTransformUtils::effectiveRotationHandleGrabRadius(const KisCoordinatesConverter *converter)
{
    QPointF handleRadiusPt = flakeToImage(converter, QPointF(rotationHandleRadius, rotationHandleRadius));
    return (handleRadiusPt.x() > handleRadiusPt.y()) ? handleRadiusPt.x() : handleRadiusPt.y();
}

qreal KisTransformUtils::scaleFromAffineMatrix(const QTransform &t) {
    return std::sqrt(t.m11() * t.m11() + t.m22() * t.m22() + t.m12() * t.m12() + t.m21() * t.m21());
}

QPointF KisTransformUtils::clipInRect(QPointF p, QRectF r)
{
    QPointF center = r.center();
    QPointF t = p - center;
    r.translate(- center);

    if (t.y() != 0) {
        if (t.x() != 0) {
            double slope = t.y() / t.x();

            if (t.x() < r.left()) {
                t.setY(r.left() * slope);
                t.setX(r.left());
            }
            else if (t.x() > r.right()) {
                t.setY(r.right() * slope);
                t.setX(r.right());
            }

            if (t.y() < r.top()) {
                t.setX(r.top() / slope);
                t.setY(r.top());
            }
            else if (t.y() > r.bottom()) {
                t.setX(r.bottom() / slope);
                t.setY(r.bottom());
            }
        }
        else {
            if (t.y() < r.top())
                t.setY(r.top());
            else if (t.y() > r.bottom())
                t.setY(r.bottom());
        }
    }
    else {
        if (t.x() < r.left())
            t.setX(r.left());
        else if (t.x() > r.right())
            t.setX(r.right());
    }

    t += center;

    return t;
}

KisTransformUtils::MatricesPack::MatricesPack(const ToolTransformArgs &args)
{
    TS = QTransform::fromTranslate(-args.originalCenter().x(), -args.originalCenter().y());
    SC = QTransform::fromScale(args.scaleX(), args.scaleY());
    S.shear(0, args.shearY()); S.shear(args.shearX(), 0);

    P.rotate(180. * args.aX() / M_PI, QVector3D(1, 0, 0));
    P.rotate(180. * args.aY() / M_PI, QVector3D(0, 1, 0));
    P.rotate(180. * args.aZ() / M_PI, QVector3D(0, 0, 1));
    projectedP = P.toTransform(args.cameraPos().z());

    QPointF translation = args.transformedCenter();
    T = QTransform::fromTranslate(translation.x(), translation.y());
}

QTransform KisTransformUtils::MatricesPack::finalTransform() const
{
    return TS * SC * S * projectedP * T;
}
