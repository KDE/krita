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
#include <QPainterPath>
#include <QTransform>
#include <KoUnit.h>
#include "tool_transform_args.h"
#include "kis_paint_device.h"
#include "kis_algebra_2d.h"
#include "transform_transaction_properties.h"

struct TransformTransactionPropertiesRegistrar {
    TransformTransactionPropertiesRegistrar() {
        qRegisterMetaType<TransformTransactionProperties>("TransformTransactionProperties");
    }
};
static TransformTransactionPropertiesRegistrar __registrar1;

struct ToolTransformArgsRegistrar {
    ToolTransformArgsRegistrar() {
        qRegisterMetaType<ToolTransformArgs>("ToolTransformArgs");
    }
};
static ToolTransformArgsRegistrar __registrar2;

struct QPainterPathRegistrar {
    QPainterPathRegistrar() {
        qRegisterMetaType<QPainterPath>("QPainterPath");
    }
};
static QPainterPathRegistrar __registrar3;


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

qreal KisTransformUtils::scaleFromPerspectiveMatrixX(const QTransform &t, const QPointF &basePt) {
    const QPointF pt = basePt + QPointF(1.0, 0);
    return kisDistance(t.map(pt), t.map(basePt));
}

qreal KisTransformUtils::scaleFromPerspectiveMatrixY(const QTransform &t, const QPointF &basePt) {
    const QPointF pt = basePt + QPointF(0, 1.0);
    return kisDistance(t.map(pt), t.map(basePt));
}

qreal KisTransformUtils::effectiveSize(const QRectF &rc) {
    return 0.5 * (rc.width() + rc.height());
}

bool KisTransformUtils::thumbnailTooSmall(const QTransform &resultThumbTransform, const QRect &originalImageRect)
{
    return KisAlgebra2D::minDimension(resultThumbTransform.mapRect(originalImageRect)) < 32;
}

QRectF handleRectImpl(qreal radius, const QTransform &t, const QRectF &limitingRect, const QPointF &basePoint, qreal *dOutX, qreal *dOutY) {
    const qreal handlesExtraScaleX =
        KisTransformUtils::scaleFromPerspectiveMatrixX(t, basePoint);
    const qreal handlesExtraScaleY =
        KisTransformUtils::scaleFromPerspectiveMatrixY(t, basePoint);

    const qreal maxD = 0.2 * KisTransformUtils::effectiveSize(limitingRect);
    const qreal dX = qMin(maxD, radius / handlesExtraScaleX);
    const qreal dY = qMin(maxD, radius / handlesExtraScaleY);

    QRectF handleRect(-0.5 * dX, -0.5 * dY, dX, dY);

    if (dOutX) {
        *dOutX = dX;
    }

    if (dOutY) {
        *dOutY = dY;
    }

    return handleRect;

}

QRectF KisTransformUtils::handleRect(qreal radius, const QTransform &t, const QRectF &limitingRect, qreal *dOutX, qreal *dOutY) {
    return handleRectImpl(radius, t, limitingRect, limitingRect.center(), dOutX, dOutY);
}

QRectF KisTransformUtils::handleRect(qreal radius, const QTransform &t, const QRectF &limitingRect, const QPointF &basePoint) {
    return handleRectImpl(radius, t, limitingRect, basePoint, 0, 0);
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

    Q_FOREACH (const QPointF &pt, points) {
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
                                      config.pixelPrecision());

        worker.prepareTransform();
        worker.setTransformedCage(config.transfPoints());
        worker.run();
    } else if (config.mode() == ToolTransformArgs::LIQUIFY && config.liquifyWorker()) {
        KoUpdaterPtr updater = helper->updater();
        //FIXME:
        Q_UNUSED(updater);

        config.liquifyWorker()->run(device);
    } else if (config.mode() == ToolTransformArgs::MESH) {
        KoUpdaterPtr updater = helper->updater();
        //FIXME:
        Q_UNUSED(updater);

        KisPaintDeviceSP srcDevice = new KisPaintDevice(*device);
        device->clear();
        config.meshTransform()->transformMesh(srcDevice, device);

    } else {
        QVector3D transformedCenter;
        KoUpdaterPtr updater1 = helper->updater();
        KoUpdaterPtr updater2 = helper->updater();

        KisTransformWorker transformWorker =
            createTransformWorker(config, device, updater1, &transformedCenter);

        transformWorker.run();

        KisPerspectiveTransformWorker::SampleType sampleType =
            config.filterId() == "NearestNeighbor" ?
            KisPerspectiveTransformWorker::NearestNeighbour :
            KisPerspectiveTransformWorker::Bilinear;

        if (config.mode() == ToolTransformArgs::FREE_TRANSFORM) {
            KisPerspectiveTransformWorker perspectiveWorker(device,
                                                            config.transformedCenter(),
                                                            config.aX(),
                                                            config.aY(),
                                                            config.cameraPos().z(),
                                                            updater2);
            perspectiveWorker.run(sampleType);
        } else if (config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT) {
            QTransform T =
                QTransform::fromTranslate(config.transformedCenter().x(),
                                          config.transformedCenter().y());

            KisPerspectiveTransformWorker perspectiveWorker(device,
                                                            T.inverted() * config.flattenedPerspectiveTransform() * T,
                                                            updater2);
            perspectiveWorker.run(sampleType);
        }
    }
}

QRect KisTransformUtils::needRect(const ToolTransformArgs &config,
                                  const QRect &rc,
                                  const QRect &srcBounds)
{
    QRect result = rc;

    if (config.mode() == ToolTransformArgs::WARP) {
        KisWarpTransformWorker worker(config.warpType(),
                                      0,
                                      config.origPoints(),
                                      config.transfPoints(),
                                      config.alpha(),
                                      0);

        result = worker.approxNeedRect(rc, srcBounds);

    } else if (config.mode() == ToolTransformArgs::CAGE) {
        KisCageTransformWorker worker(0,
                                      config.origPoints(),
                                      0,
                                      config.pixelPrecision());
        worker.setTransformedCage(config.transfPoints());
        result = worker.approxNeedRect(rc, srcBounds);
    } else if (config.mode() == ToolTransformArgs::LIQUIFY) {
        result = config.liquifyWorker() ?
            config.liquifyWorker()->approxNeedRect(rc, srcBounds) : rc;
    } else if (config.mode() == ToolTransformArgs::MESH) {
        result = config.meshTransform()->approxNeedRect(rc);
    } else {
        KIS_ASSERT_RECOVER_NOOP(0 && "this works for non-affine transformations only!");
    }

    return result;
}

QRect KisTransformUtils::changeRect(const ToolTransformArgs &config,
                                    const QRect &rc)
{
    QRect result = rc;

    if (config.mode() == ToolTransformArgs::WARP) {
        KisWarpTransformWorker worker(config.warpType(),
                                      0,
                                      config.origPoints(),
                                      config.transfPoints(),
                                      config.alpha(),
                                      0);

        result = worker.approxChangeRect(rc);

    } else if (config.mode() == ToolTransformArgs::CAGE) {
        KisCageTransformWorker worker(0,
                                      config.origPoints(),
                                      0,
                                      config.pixelPrecision());

        worker.setTransformedCage(config.transfPoints());
        result = worker.approxChangeRect(rc);
    } else if (config.mode() == ToolTransformArgs::LIQUIFY) {
        result = config.liquifyWorker() ?
            config.liquifyWorker()->approxChangeRect(rc) : rc;
    } else if (config.mode() == ToolTransformArgs::MESH) {
        result = config.meshTransform()->approxChangeRect(rc);

    } else {
        KIS_ASSERT_RECOVER_NOOP(0 && "this works for non-affine transformations only!");
    }

    return result;
}

KisTransformUtils::AnchorHolder::AnchorHolder(bool enabled, ToolTransformArgs *config)
    : m_enabled(enabled),
      m_config(config)
{
    if (!m_enabled) return;

    m_staticPoint = m_config->originalCenter() + m_config->rotationCenterOffset();

    const KisTransformUtils::MatricesPack m(*m_config);
    m_oldStaticPointInView = m.finalTransform().map(m_staticPoint);
}

KisTransformUtils::AnchorHolder::~AnchorHolder() {
    if (!m_enabled) return;

    const KisTransformUtils::MatricesPack m(*m_config);
    const QPointF newStaticPointInView = m.finalTransform().map(m_staticPoint);

    const QPointF diff = m_oldStaticPointInView - newStaticPointInView;

    m_config->setTransformedCenter(m_config->transformedCenter() + diff);
}

void KisTransformUtils::setDefaultWarpPoints(int pointsPerLine,
                                             const TransformTransactionProperties *transaction,
                                             ToolTransformArgs *config)
{
    static const int DEFAULT_POINTS_PER_LINE = 3;

    if (pointsPerLine < 0) {
        pointsPerLine = DEFAULT_POINTS_PER_LINE;
    }

    int nbPoints = pointsPerLine * pointsPerLine;
    QVector<QPointF> origPoints(nbPoints);
    QVector<QPointF> transfPoints(nbPoints);
    qreal gridSpaceX, gridSpaceY;

    if (nbPoints == 1) {
        //there is actually no grid
        origPoints[0] = transaction->originalCenterGeometric();
        transfPoints[0] = transaction->originalCenterGeometric();
    }
    else if (nbPoints > 1) {
        gridSpaceX = transaction->originalRect().width() / (pointsPerLine - 1);
        gridSpaceY = transaction->originalRect().height() / (pointsPerLine - 1);
        double y = transaction->originalRect().top();
        for (int i = 0; i < pointsPerLine; ++i) {
            double x = transaction->originalRect().left();
            for (int j = 0 ; j < pointsPerLine; ++j) {
                origPoints[i * pointsPerLine + j] = QPointF(x, y);
                transfPoints[i * pointsPerLine + j] = QPointF(x, y);
                x += gridSpaceX;
            }
            y += gridSpaceY;
        }
    }

    config->setDefaultPoints(nbPoints > 0);
    config->setPoints(origPoints, transfPoints);
}

ToolTransformArgs KisTransformUtils::resetArgsForMode(ToolTransformArgs::TransformMode mode,
                                                      const QString &filterId,
                                                      const TransformTransactionProperties &transaction)
{
    ToolTransformArgs args;

    args.setOriginalCenter(transaction.originalCenterGeometric());
    args.setTransformedCenter(transaction.originalCenterGeometric());
    args.setFilterId(filterId);

    if (mode == ToolTransformArgs::FREE_TRANSFORM) {
        args.setMode(ToolTransformArgs::FREE_TRANSFORM);
    } else if (mode == ToolTransformArgs::WARP) {
        args.setMode(ToolTransformArgs::WARP);
        KisTransformUtils::setDefaultWarpPoints(-1, &transaction, &args);
        args.setEditingTransformPoints(false);
    } else if (mode == ToolTransformArgs::CAGE) {
        args.setMode(ToolTransformArgs::CAGE);
        args.setEditingTransformPoints(true);
    } else if (mode == ToolTransformArgs::LIQUIFY) {
        args.setMode(ToolTransformArgs::LIQUIFY);
        const QRect srcRect = transaction.originalRect().toAlignedRect();
        if (!srcRect.isEmpty()) {
            args.initLiquifyTransformMode(srcRect);
        }
    } else if (mode == ToolTransformArgs::MESH) {
        args.setMode(ToolTransformArgs::MESH);
        const QRect srcRect = transaction.originalRect().toAlignedRect();
        if (!srcRect.isEmpty()) {
            *args.meshTransform() = KisBezierTransformMesh(QRectF(srcRect));
        }
    } else if (mode == ToolTransformArgs::PERSPECTIVE_4POINT) {
        args.setMode(ToolTransformArgs::PERSPECTIVE_4POINT);
    }

    return args;
}
