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

#include "kis_constrained_rect.h"

#include <cmath>
#include "kis_debug.h"
#include "kis_algebra_2d.h"


KisConstrainedRect::KisConstrainedRect()
    : m_centered(false),
      m_canGrow(true),
      m_ratio(1.0),
      m_widthLocked(false),
      m_heightLocked(false),
      m_ratioLocked(false)
{
}

KisConstrainedRect::~KisConstrainedRect()
{
}

void KisConstrainedRect::setRectInitial(const QRect &rect)
{
    m_rect = rect;
    if (!ratioLocked()) {
        storeRatioSafe(m_rect.size());
    }
    emit sigValuesChanged();
}

void KisConstrainedRect::setCropRect(const QRect &cropRect)
{
    m_cropRect = cropRect;
}

bool KisConstrainedRect::centered() const {
    return m_centered;
}
void KisConstrainedRect::setCentered(bool value) {
    m_centered = value;
}

bool KisConstrainedRect::canGrow() const {
    return m_canGrow;
}
void KisConstrainedRect::setCanGrow(bool value) {
    m_canGrow = value;
}

QRect KisConstrainedRect::rect() const {
    return m_rect.normalized();
}

qreal KisConstrainedRect::ratio() const {
    return qAbs(m_ratio);
}

void KisConstrainedRect::moveHandle(HandleType handle, const QPoint &offset, const QRect &oldRect)
{
    const QSize oldSize = oldRect.size();
    QSize newSize = oldSize;
    QPoint newOffset = oldRect.topLeft();

    int xSizeCoeff = 1;
    int ySizeCoeff = 1;

    qreal xOffsetFromSizeChange = 1.0;
    qreal yOffsetFromSizeChange = 1.0;

    int baseSizeCoeff = 1;

    bool useMoveOnly = false;

    switch (handle) {
    case UpperLeft:
        xSizeCoeff = -1;
        ySizeCoeff = -1;
        xOffsetFromSizeChange = -1.0;
        yOffsetFromSizeChange = -1.0;
        break;
    case UpperRight:
        xSizeCoeff =  1;
        ySizeCoeff = -1;
        xOffsetFromSizeChange =  0.0;
        yOffsetFromSizeChange = -1.0;
        break;
    case Creation:
        baseSizeCoeff = 0;
        /* Falls through */
    case LowerRight:
        xSizeCoeff =  1;
        ySizeCoeff =  1;
        xOffsetFromSizeChange =  0.0;
        yOffsetFromSizeChange =  0.0;
        break;
    case LowerLeft:
        xSizeCoeff = -1;
        ySizeCoeff =  1;
        xOffsetFromSizeChange = -1.0;
        yOffsetFromSizeChange =  0.0;
        break;
    case Upper:
        xSizeCoeff =  0;
        ySizeCoeff = -1;
        xOffsetFromSizeChange = -0.5;
        yOffsetFromSizeChange = -1.0;
        break;
    case Right:
        xSizeCoeff =  1;
        ySizeCoeff =  0;
        xOffsetFromSizeChange =  0.0;
        yOffsetFromSizeChange = -0.5;
        break;
    case Lower:
        xSizeCoeff =  0;
        ySizeCoeff =  1;
        xOffsetFromSizeChange = -0.5;
        yOffsetFromSizeChange =  0.0;
        break;
    case Left:
        xSizeCoeff = -1;
        ySizeCoeff =  0;
        xOffsetFromSizeChange = -1.0;
        yOffsetFromSizeChange = -0.5;
        break;
    case Inside:
        useMoveOnly = true;
        break;
    case None: // should never happen
        break;
    }

    if (!useMoveOnly) {
        const int centeringSizeCoeff = m_centered ? 2 : 1;
        if (m_centered) {
            xOffsetFromSizeChange = -0.5;
            yOffsetFromSizeChange = -0.5;
        }


        QSize sizeDiff(offset.x() * xSizeCoeff * centeringSizeCoeff,
                       offset.y() * ySizeCoeff * centeringSizeCoeff);

        QSize tempSize = baseSizeCoeff * oldSize + sizeDiff;
        bool widthPreferrable = qAbs(tempSize.width()) > qAbs(tempSize.height() * m_ratio);

        if (ratioLocked() && ((widthPreferrable && xSizeCoeff != 0) || ySizeCoeff == 0)) {
            newSize.setWidth(tempSize.width());
            newSize.setHeight(heightFromWidthUnsignedRatio(newSize.width(), m_ratio, tempSize.height()));
        } else if (ratioLocked() && ((!widthPreferrable && ySizeCoeff != 0) || xSizeCoeff == 0)) {
            newSize.setHeight(tempSize.height());
            newSize.setWidth(widthFromHeightUnsignedRatio(newSize.height(), m_ratio, tempSize.width()));
        } else if (widthLocked() && heightLocked()) {
            newSize.setWidth(KisAlgebra2D::copysign(newSize.width(), tempSize.width()));
            newSize.setHeight(KisAlgebra2D::copysign(newSize.height(), tempSize.height()));
        } else if (widthLocked()) {
            newSize.setWidth(KisAlgebra2D::copysign(newSize.width(), tempSize.width()));
            newSize.setHeight(tempSize.height());
            storeRatioSafe(newSize);
        } else if (heightLocked()) {
            newSize.setHeight(KisAlgebra2D::copysign(newSize.height(), tempSize.height()));
            newSize.setWidth(tempSize.width());
            storeRatioSafe(newSize);
        } else {
            newSize = baseSizeCoeff * oldSize + sizeDiff;
            storeRatioSafe(newSize);
        }

        QSize realSizeDiff = newSize - baseSizeCoeff * oldSize;
        QPoint offsetDiff(realSizeDiff.width() * xOffsetFromSizeChange,
                          realSizeDiff.height() * yOffsetFromSizeChange);

        newOffset = oldRect.topLeft() + offsetDiff;
    } else {
        newOffset = oldRect.topLeft() + offset;
    }

    m_rect = QRect(newOffset, newSize);

    if (!m_canGrow) {
        m_rect &= m_cropRect;
    }

    emit sigValuesChanged();
}

QPointF KisConstrainedRect::handleSnapPoint(HandleType handle, const QPointF &cursorPos)
{
    QPointF snapPoint = cursorPos;

    switch (handle) {
    case UpperLeft:
        snapPoint = m_rect.topLeft();
        break;
    case UpperRight:
        snapPoint = m_rect.topRight() + QPointF(1, 0);
        break;
    case Creation:
        break;
    case LowerRight:
        snapPoint = m_rect.bottomRight() + QPointF(1, 1);
        break;
    case LowerLeft:
        snapPoint = m_rect.bottomLeft() + QPointF(0, 1);
        break;
    case Upper:
        snapPoint.ry() = m_rect.y();
        break;
    case Right:
        snapPoint.rx() = m_rect.right() + 1;
        break;
    case Lower:
        snapPoint.ry() = m_rect.bottom() + 1;
        break;
    case Left:
        snapPoint.rx() = m_rect.x();
        break;
    case Inside:
        break;
    case None: // should never happen
        break;
    }

    return snapPoint;
}

void KisConstrainedRect::normalize()
{
    setRectInitial(m_rect.normalized());
}

void KisConstrainedRect::setOffset(const QPoint &offset)
{
    QRect newRect = m_rect;
    newRect.moveTo(offset);

    if (!m_canGrow) {
        newRect &= m_cropRect;
    }

    if (!newRect.isEmpty()) {
        m_rect = newRect;
    }

    emit sigValuesChanged();
}

void KisConstrainedRect::setRatio(qreal value) {
    KIS_ASSERT_RECOVER_RETURN(value >= 0);

    const qreal eps = 1e-7;
    const qreal invEps = 1.0 / eps;

    if (value < eps || value > invEps) {
        emit sigValuesChanged();
        return;
    }

    const QSize oldSize = m_rect.size();
    QSize newSize = oldSize;

    if (widthLocked() && heightLocked()) {
        setHeightLocked(false);
    }

    m_ratio = value;

    if (!widthLocked() && !heightLocked()) {
        int area = oldSize.width() * oldSize.height();
        newSize.setWidth(qRound(std::sqrt(area * m_ratio)));
        newSize.setHeight(qRound(newSize.width() / m_ratio));
    } else if (widthLocked()) {
        newSize.setHeight(newSize.width() / m_ratio);
    } else if (heightLocked()) {
        newSize.setWidth(newSize.height() * m_ratio);
    }

    assignNewSize(newSize);
}

void KisConstrainedRect::setWidth(int value)
{
    KIS_ASSERT_RECOVER_RETURN(value >= 0);

    const QSize oldSize = m_rect.size();
    QSize newSize = oldSize;

    if (ratioLocked()) {
        newSize.setWidth(value);
        newSize.setHeight(newSize.width() / m_ratio);
    } else {
        newSize.setWidth(value);
        storeRatioSafe(newSize);
    }

    assignNewSize(newSize);
}

void KisConstrainedRect::setHeight(int value)
{
    KIS_ASSERT_RECOVER_RETURN(value >= 0);

    const QSize oldSize = m_rect.size();
    QSize newSize = oldSize;

    if (ratioLocked()) {
        newSize.setHeight(value);
        newSize.setWidth(newSize.height() * m_ratio);
    } else {
        newSize.setHeight(value);
        storeRatioSafe(newSize);
    }

    assignNewSize(newSize);
}

void KisConstrainedRect::assignNewSize(const QSize &newSize)
{
    if (!m_centered) {
        m_rect.setSize(newSize);
    } else {
        QSize sizeDiff = newSize - m_rect.size();
        m_rect.translate(-qRound(sizeDiff.width() / 2.0), -qRound(sizeDiff.height() / 2.0));
        m_rect.setSize(newSize);
    }

    if (!m_canGrow) {
        m_rect &= m_cropRect;
    }

    emit sigValuesChanged();
}

void KisConstrainedRect::storeRatioSafe(const QSize &newSize)
{
    m_ratio = qAbs(qreal(newSize.width()) / newSize.height());
}

int KisConstrainedRect::widthFromHeightUnsignedRatio(int height, qreal ratio, int oldWidth) const
{
    int newWidth = qRound(height * ratio);
    return KisAlgebra2D::copysign(newWidth, oldWidth);
}

int KisConstrainedRect::heightFromWidthUnsignedRatio(int width, qreal ratio, int oldHeight) const
{
    int newHeight = qRound(width / ratio);
    return KisAlgebra2D::copysign(newHeight, oldHeight);
}

bool KisConstrainedRect::widthLocked() const {
    return m_widthLocked;
}
bool KisConstrainedRect::heightLocked() const {
    return m_heightLocked;
}
bool KisConstrainedRect::ratioLocked() const {
    return m_ratioLocked;
}

void KisConstrainedRect::setWidthLocked(bool value) {
    m_widthLocked = value;
    m_ratioLocked &= !(m_widthLocked || m_heightLocked);

    emit sigLockValuesChanged();
}

void KisConstrainedRect::setHeightLocked(bool value) {
    m_heightLocked = value;
    m_ratioLocked &= !(m_widthLocked || m_heightLocked);

    emit sigLockValuesChanged();
}

void KisConstrainedRect::setRatioLocked(bool value) {
    m_ratioLocked = value;

    m_widthLocked &= !m_ratioLocked;
    m_heightLocked &= !m_ratioLocked;

    emit sigLockValuesChanged();
}

