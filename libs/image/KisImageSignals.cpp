/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisImageSignals.h"

#include "kis_node.h"

/*******************************************************************************/
/*                       ComplexSizeChangedSignal                              */
/*******************************************************************************/

ComplexSizeChangedSignal::ComplexSizeChangedSignal() {}

ComplexSizeChangedSignal::ComplexSizeChangedSignal(QPointF _oldStillPoint, QPointF _newStillPoint)
    : oldStillPoint(_oldStillPoint),
      newStillPoint(_newStillPoint)
{
}

ComplexSizeChangedSignal::ComplexSizeChangedSignal(const QRect &portionOfOldImage, const QSize &transformedIntoImageOfSize)
{
    oldStillPoint = QRectF(portionOfOldImage).center();
    newStillPoint = QRectF(QPointF(), QSizeF(transformedIntoImageOfSize)).center();
}

ComplexSizeChangedSignal ComplexSizeChangedSignal::inverted() const {
    return ComplexSizeChangedSignal(newStillPoint, oldStillPoint);
}


/*******************************************************************************/
/*                       ComplexNodeReselectionSignal                          */
/*******************************************************************************/

ComplexNodeReselectionSignal::ComplexNodeReselectionSignal() {}

ComplexNodeReselectionSignal::ComplexNodeReselectionSignal(KisNodeSP _newActiveNode, KisNodeList _newSelectedNodes, KisNodeSP _oldActiveNode, KisNodeList _oldSelectedNodes)
    : newActiveNode(_newActiveNode),
      newSelectedNodes(_newSelectedNodes),
      oldActiveNode(_oldActiveNode),
      oldSelectedNodes(_oldSelectedNodes)
{
}

ComplexNodeReselectionSignal ComplexNodeReselectionSignal::inverted() const {
    return ComplexNodeReselectionSignal(oldActiveNode, oldSelectedNodes, newActiveNode, newSelectedNodes);
}


/*******************************************************************************/
/*                       KisImageSignalType                                    */
/*******************************************************************************/

KisImageSignalType::KisImageSignalType() {}

KisImageSignalType::KisImageSignalType(KisImageSignalTypeEnum _id)
    : id(_id)
{
}

KisImageSignalType::KisImageSignalType(ComplexSizeChangedSignal signal)
    : id(SizeChangedSignal),
      sizeChangedSignal(signal)
{
}

KisImageSignalType::KisImageSignalType(ComplexNodeReselectionSignal signal)
    : id(NodeReselectionRequestSignal),
      nodeReselectionSignal(signal)
{
}

KisImageSignalType KisImageSignalType::inverted() const {
    KisImageSignalType t;
    t.id = id;
    t.sizeChangedSignal = sizeChangedSignal.inverted();
    t.nodeReselectionSignal = nodeReselectionSignal.inverted();
    return t;
}
