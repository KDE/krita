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
#include <KoUnit.h>
#include "tool_transform_args.h"


const int KisTransformUtils::rotationHandleVisualRadius = 12;
const int KisTransformUtils::rotationHandleRadius = 8;
const int KisTransformUtils::handleVisualRadius = 12;
const int KisTransformUtils::handleRadius = 8;


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
    return KoUnit::approxTransformScale(t);
}

qreal KisTransformUtils::scaleFromPerspectiveMatrix(const QTransform &t, const QPointF &basePt) {
    const QRectF testRect(basePt, QSizeF(1.0, 1.0));
    QRectF resultRect = t.mapRect(testRect);

    return 0.5 * (resultRect.width(), resultRect.height());
}

qreal KisTransformUtils::effectiveSize(const QRectF &rc) {
    return 0.5 * (rc.width(), rc.height());
}

QRectF KisTransformUtils::handleRect(qreal radius, const QTransform &t, const QRectF &limitingRect, qreal *dOut) {
    qreal handlesExtraScale =
        scaleFromPerspectiveMatrix(t, limitingRect.center());

    const qreal maxD = 0.2 * effectiveSize(limitingRect);
    const qreal d = qMin(maxD, radius / handlesExtraScale);

    QRectF handleRect(-0.5 * d, -0.5 * d, d, d);

    if (dOut) {
        *dOut = d;
    }

    return handleRect;
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

    if (args.mode() == ToolTransformArgs::FREE_TRANSFORM) {
        P.rotate(180. * args.aX() / M_PI, QVector3D(1, 0, 0));
        P.rotate(180. * args.aY() / M_PI, QVector3D(0, 1, 0));
        P.rotate(180. * args.aZ() / M_PI, QVector3D(0, 0, 1));
        projectedP = P.toTransform(args.cameraPos().z());
    } else if (args.mode() == ToolTransformArgs::PERSPECTIVE_4POINT) {
        projectedP = args.flattenedPerspectiveTransform();
        P = QMatrix4x4(projectedP);
    }

    QPointF translation = args.transformedCenter();
    T = QTransform::fromTranslate(translation.x(), translation.y());
}

QTransform KisTransformUtils::MatricesPack::finalTransform() const
{
    return TS * SC * S * projectedP * T;
}

bool KisTransformUtils::checkImageTooBig(const QRectF &bounds, const MatricesPack &m)
{
    bool imageTooBig = false;

    QMatrix4x4 unprojectedMatrix = QMatrix4x4(m.T) * m.P * QMatrix4x4(m.TS * m.SC * m.S);
    QVector<QPointF> points;
    points << bounds.topLeft();
    points << bounds.topRight();
    points << bounds.bottomRight();
    points << bounds.bottomLeft();

    foreach (const QPointF &pt, points) {
        QVector4D v(pt.x(), pt.y(), 0, 1);

        v = unprojectedMatrix * v;
        qreal z = v.z() / v.w();

        imageTooBig = z > 1024.0;

        if (imageTooBig) {
            break;
        }
    }

    return imageTooBig;
}

#include <kis_transform_worker.h>
#include <kis_perspectivetransform_worker.h>
#include <kis_warptransform_worker.h>
#include <kis_cage_transform_worker.h>
#include <kis_liquify_transform_worker.h>

KisTransformWorker KisTransformUtils::createTransformWorker(const ToolTransformArgs &config,
                                                            KisPaintDeviceSP device,
                                                            KoUpdaterPtr updater,
                                                            QVector3D *transformedCenter /* OUT */)
{
    {
        KisTransformWorker t(0,
                             config.scaleX(), config.scaleY(),
                             config.shearX(), config.shearY(),
                             config.originalCenter().x(),
                             config.originalCenter().y(),
                             config.aZ(),
                             0, // set X and Y translation
                             0, // to null for calculation
                             0,
                             config.filter());

        *transformedCenter = QVector3D(t.transform().map(config.originalCenter()));
    }

    QPointF translation = config.transformedCenter() - (*transformedCenter).toPointF();

    KisTransformWorker transformWorker(device,
                                       config.scaleX(), config.scaleY(),
                                       config.shearX(), config.shearY(),
                                       config.originalCenter().x(),
                                       config.originalCenter().y(),
                                       config.aZ(),
                                       (int)(translation.x()),
                                       (int)(translation.y()),
                                       updater,
                                       config.filter());

    return transformWorker;
}

void KisTransformUtils::transformDevice(const ToolTransformArgs &config,
                                        KisPaintDeviceSP device,
                                        KisProcessingVisitor::ProgressHelper *helper)
{
    if (config.mode() == ToolTransformArgs::WARP) {
        KoUpdaterPtr updater = helper->updater();

        KisWarpTransformWorker worker(config.warpType(),
                                      device,
                                      config.origPoints(),
                                      config.transfPoints(),
                                      config.alpha(),
                                      updater);
        worker.run();
    } else if (config.mode() == ToolTransformArgs::CAGE) {
        KoUpdaterPtr updater = helper->updater();

        KisCageTransformWorker worker(device,
                                      config.origPoints(),
                                      updater,
                                      8);

        worker.prepareTransform();
        worker.setTransformedCage(config.transfPoints());
        worker.run();
    } else if (config.mode() == ToolTransformArgs::LIQUIFY) {
        KoUpdaterPtr updater = helper->updater();
        //FIXME:
        Q_UNUSED(updater);

        config.liquifyWorker()->run(device);
    } else {
        QVector3D transformedCenter;
        KoUpdaterPtr updater1 = helper->updater();
        KoUpdaterPtr updater2 = helper->updater();

        KisTransformWorker transformWorker =
            createTransformWorker(config, device, updater1, &transformedCenter);

        transformWorker.run();

        if (config.mode() == ToolTransformArgs::FREE_TRANSFORM) {
            KisPerspectiveTransformWorker perspectiveWorker(device,
                                                            config.transformedCenter(),
                                                            config.aX(),
                                                            config.aY(),
                                                            config.cameraPos().z(),
                                                            updater2);
            perspectiveWorker.run();
        } else if (config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT) {
            QTransform T =
                QTransform::fromTranslate(config.transformedCenter().x(),
                                          config.transformedCenter().y());

            KisPerspectiveTransformWorker perspectiveWorker(device,
                                                            T.inverted() * config.flattenedPerspectiveTransform() * T,
                                                            updater2);
            perspectiveWorker.run();
        }
    }
}
