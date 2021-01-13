/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class KisSavedMacroCommand;
class KisStrokeUndoFacade;
class KisStrokeJobData;

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

    static void transformDevice(const ToolTransformArgs &config,
                                KisPaintDeviceSP srcDevice,
                                KisPaintDeviceSP dstDevice,
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

    static bool shouldRestartStrokeOnModeChange(ToolTransformArgs::TransformMode oldMode,
                                                ToolTransformArgs::TransformMode newMode,
                                                KisNodeList processedNodes);

    static void transformAndMergeDevice(const ToolTransformArgs &config,
                                        KisPaintDeviceSP src,
                                        KisPaintDeviceSP dst,
                                        KisProcessingVisitor::ProgressHelper *helper);

    static void postProcessToplevelCommand(KUndo2Command *command,
                                           const ToolTransformArgs &args,
                                           KisNodeSP rootNode,
                                           KisNodeList processedNodes,
                                           const KisSavedMacroCommand *overriddenCommand);

    static bool fetchArgsFromCommand(const KUndo2Command *command,
                                     ToolTransformArgs *args,
                                     KisNodeSP *rootNode,
                                     KisNodeList *transformedNodes);

    static KisNodeSP tryOverrideRootToTransformMask(KisNodeSP root);

    static QList<KisNodeSP> fetchNodesList(ToolTransformArgs::TransformMode mode, KisNodeSP root, bool recursive);
    static bool tryInitArgsFromNode(KisNodeSP node, ToolTransformArgs *args);
    static bool tryFetchArgsFromCommandAndUndo(ToolTransformArgs *outArgs, ToolTransformArgs::TransformMode mode, KisNodeSP currentNode, KisNodeList selectedNodes, KisStrokeUndoFacade *undoFacade, QVector<KisStrokeJobData *> *undoJobs, const KisSavedMacroCommand **overriddenCommand);

};

#endif /* __KIS_TRANSFORM_UTILS_H */
