/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __TRANSFORM_TRANSACTION_PROPERTIES_H
#define __TRANSFORM_TRANSACTION_PROPERTIES_H

#include <QRectF>
#include <QPointF>

class ToolTransformArgs;


class TransformTransactionProperties
{
public:
    TransformTransactionProperties()
        : m_editWarpPoints(false)
    {
    }

    TransformTransactionProperties(const QRectF &originalRect, ToolTransformArgs *currentConfig)
        : m_originalRect(originalRect),
          m_currentConfig(currentConfig),
          m_editWarpPoints(false)
    {
    }

    qreal originalHalfWidth() const {
        return m_originalRect.width() / 2.0;
    }

    qreal originalHalfHeight() const {
        return m_originalRect.height() / 2.0;
    }

    QRectF originalRect() const {
        return m_originalRect;
    }

    QPointF originalCenter() const {
        return m_originalRect.center();
    }

    QPointF originalTopLeft() const {
        return m_originalRect.topLeft();
    }

    QPointF originalBottomLeft() const {
        return m_originalRect.bottomLeft();
    }

    QPointF originalBottomRight() const {
        return m_originalRect.bottomRight();
    }

    QPointF originalTopRight() const {
        return m_originalRect.topRight();
    }

    QPointF originalMiddleLeft() const {
        return QPointF(m_originalRect.left(), (m_originalRect.top() + m_originalRect.bottom()) / 2.0);
    }

    QPointF originalMiddleRight() const {
        return QPointF(m_originalRect.right(), (m_originalRect.top() + m_originalRect.bottom()) / 2.0);
    }

    QPointF originalMiddleTop() const {
        return QPointF((m_originalRect.left() + m_originalRect.right()) / 2.0, m_originalRect.top());
    }

    QPointF originalMiddleBottom() const {
        return QPointF((m_originalRect.left() + m_originalRect.right()) / 2.0, m_originalRect.bottom());
    }

    QPoint originalTopLeftAligned() const {
        return m_originalRect.toAlignedRect().topLeft();
    }

    QPoint originalBottomRightAligned() const {
        return m_originalRect.toAlignedRect().bottomRight();
    }

    bool editWarpPoints() const {
        return m_editWarpPoints;
    }

    void setEditWarpPoints(bool value) {
        m_editWarpPoints = value;
    }

    const ToolTransformArgs* currentConfig() const {
        return m_currentConfig;
    }

private:
    /**
     * Information about the original selected rect
     * (before any transformations)
     */
    QRectF m_originalRect;
    ToolTransformArgs *m_currentConfig;
    bool m_editWarpPoints;
};

#endif /* __TRANSFORM_TRANSACTION_PROPERTIES_H */
