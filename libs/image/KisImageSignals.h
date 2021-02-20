/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGESIGNALS_H
#define KISIMAGESIGNALS_H

#include <QVector>
#include <QRectF>
#include "kritaimage_export.h"
#include "kis_types.h"

enum KisImageSignalTypeEnum {
    LayersChangedSignal,
    ModifiedWithoutUndoSignal,
    SizeChangedSignal,
    ProfileChangedSignal,
    ColorSpaceChangedSignal,
    ResolutionChangedSignal,
    NodeReselectionRequestSignal
};

/**
 * A special signal which handles stillPoint capabilities of the image
 *
 * \see KisImage::sigSizeChanged()
 */
struct KRITAIMAGE_EXPORT ComplexSizeChangedSignal {
    ComplexSizeChangedSignal();
    ComplexSizeChangedSignal(QPointF _oldStillPoint, QPointF _newStillPoint);

    /**
     * A helper method calculating the still points from image areas
     * we process. It works as if the source image was "cropped" by \p
     * portionOfOldImage, and this portion formed the new image of size
     * \p transformedIntoImageOfSize.
     *
     * Note, that \p portionOfTheImage may be equal to the image bounds().
     */
    ComplexSizeChangedSignal(const QRect &portionOfOldImage, const QSize &transformedIntoImageOfSize);

    ComplexSizeChangedSignal inverted() const;

    QPointF oldStillPoint;
    QPointF newStillPoint;
};

/**
 * A special signal which handles emitting signals for node reselection
 *
 * \see KisImage::sigRequestNodeReselection()
 */
struct KRITAIMAGE_EXPORT ComplexNodeReselectionSignal {
    ComplexNodeReselectionSignal();
    ComplexNodeReselectionSignal(KisNodeSP _newActiveNode, KisNodeList _newSelectedNodes,
                                 KisNodeSP _oldActiveNode = KisNodeSP(), KisNodeList _oldSelectedNodes = KisNodeList());

    ComplexNodeReselectionSignal inverted() const;

    KisNodeSP newActiveNode;
    KisNodeList newSelectedNodes;
    KisNodeSP oldActiveNode;
    KisNodeList oldSelectedNodes;
};

struct KRITAIMAGE_EXPORT KisImageSignalType {
    KisImageSignalType();
    KisImageSignalType(KisImageSignalTypeEnum _id);
    KisImageSignalType(ComplexSizeChangedSignal signal);
    KisImageSignalType(ComplexNodeReselectionSignal signal);

    KisImageSignalType inverted() const;

    KisImageSignalTypeEnum id;
    ComplexSizeChangedSignal sizeChangedSignal;
    ComplexNodeReselectionSignal nodeReselectionSignal;
};

typedef QVector<KisImageSignalType> KisImageSignalVector;

#endif // KISIMAGESIGNALS_H
