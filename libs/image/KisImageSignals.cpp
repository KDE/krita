/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
