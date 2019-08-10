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

#ifndef __KIS_TRANSFORM_UTILS_H
#define __KIS_TRANSFORM_UTILS_H

#include <QtGlobal>

#include "kis_coordinates_converter.h"

#include <QTransform>
#include <QMatrix4x4>
#include <kis_processing_visitor.h>
#include <limits>

// for kisSquareDistance only
#include "kis_global.h"

#include "tool_transform_args.h"

class ToolTransformArgs;
class KisTransformWorker;
class TransformTransactionProperties;

class KisTransformUtils
{
public:

    static const int rotationHandleVisualRadius;
    static const int handleVisualRadius;
    static const int handleRadius;
    static const int rotationHandleRadius;

    template <class T>
    static T flakeToImage(const KisCoordinatesConverter *converter, T object) {
        return converter->documentToImage(converter->flakeToDocument(object));
    }

    template <class T>
    static T imageToFlake(const KisCoordinatesConverter *converter, T object) {
        return converter->documentToFlake(converter->imageToDocument(object));
    }

    static QTransform imageToFlakeTransform(const KisCoordinatesConverter *converter);
    static qreal effectiveHandleGrabRadius(const KisCoordinatesConverter *converter);

    static qreal effectiveRotationHandleGrabRadius(const KisCoordinatesConverter *converter);

    static qreal scaleFromAffineMatrix(const QTransform &t);
    static qreal scaleFromPerspectiveMatrixX(const QTransform &t, const QPointF &basePt);
    static qreal scaleFromPerspectiveMatrixY(const QTransform &t, const QPointF &basePt);
    static qreal effectiveSize(const QRectF &rc);
    static bool thumbnailTooSmall(const QTransform &resultThumbTransform, const QRect &originalImageRect);

    static QRectF handleRect(qreal radius, const QTransform &t, const QRectF &limitingRect, qreal *dOutX, qreal *dOutY);
    static QRectF handleRect(qreal radius, const QTransform &t, const QRectF &limitingRect, const QPointF &basePoint);

    static QPointF clipInRect(QPointF p, QRectF r);

    struct MatricesPack
    {
        MatricesPack(const ToolTransformArgs &args);

        QTransform TS;
        QTransform SC;
        QTransform S;
        QMatrix4x4 P;
        QTransform projectedP;
        QTransform T;

        // the final transformation looks like
        // transform = TS * SC * S * projectedP * T
        QTransform finalTransform() const;
    };

    static bool checkImageTooBig(const QRectF &bounds, const MatricesPack &m);

    static KisTransformWorker createTransformWorker(const ToolTransformArgs &config,
                                                    KisPaintDeviceSP device,
                                                    KoUpdaterPtr updater,
                                                    QVector3D *transformedCenter /* OUT */);

    static void transformDevice(const ToolTransformArgs &config,
                                KisPaintDeviceSP device,
                                KisProcessingVisitor::ProgressHelper *helper);

    static QRect needRect(const ToolTransformArgs &config,
                          const QRect &rc,
                          const QRect &srcBounds);

    static QRect changeRect(const ToolTransformArgs &config,
                            const QRect &rc);

    template<typename Function>
    class HandleChooser {
    public:
        HandleChooser(const QPointF &cursorPos, Function defaultFunction)
            : m_cursorPos(cursorPos),
              m_minDistance(std::numeric_limits<qreal>::max()),
              m_function(defaultFunction)
        {
        }

        bool addFunction(const QPointF &pt, qreal radius, Function function) {
            bool result = false;
            qreal distance = kisSquareDistance(pt, m_cursorPos);

            if (distance < pow2(radius) && distance < m_minDistance) {
                m_minDistance = distance;
                m_function = function;
                result = true;
            }

            return result;
        }

        Function function() const {
            return m_function;
        }

    private:
        QPointF m_cursorPos;
        qreal m_minDistance;
        Function m_function;
    };

    /**
     * A special class that ensures that the view position of the anchor point of the
     * transformation is unchanged during the lifetime of the object. On destruction
     * of the keeper the position of the anchor point will be restored.
     */
    struct AnchorHolder {
        AnchorHolder(bool enabled, ToolTransformArgs *config);
        ~AnchorHolder();

    private:
        bool m_enabled;
        ToolTransformArgs *m_config;
        QPointF m_staticPoint;
        QPointF m_oldStaticPointInView;

    };

    static void setDefaultWarpPoints(int pointsPerLine,
                                     const TransformTransactionProperties *transaction,
                                     ToolTransformArgs *config);

    static ToolTransformArgs resetArgsForMode(ToolTransformArgs::TransformMode mode,
                                              const QString &filterId,
                                              const TransformTransactionProperties &transaction);

};

#endif /* __KIS_TRANSFORM_UTILS_H */
