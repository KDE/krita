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
#include "kis_node.h"
#include "kis_layer_utils.h"
#include "kis_external_layer_iface.h"

class ToolTransformArgs;

class TransformTransactionProperties
{
public:
    TransformTransactionProperties()
    {
    }

TransformTransactionProperties(const QRectF &originalRect,
                               ToolTransformArgs *currentConfig,
                               KisNodeSP rootNode,
                               const QList<KisNodeSP> &transformedNodes)
        : m_originalRect(originalRect),
          m_currentConfig(currentConfig),
          m_rootNode(rootNode),
          m_transformedNodes(transformedNodes),
          m_shouldAvoidPerspectiveTransform(false)
    {
        m_hasInvisibleNodes = false;
        Q_FOREACH (KisNodeSP node, transformedNodes) {
            if (KisExternalLayer *extLayer = dynamic_cast<KisExternalLayer*>(node.data())) {
                if (!extLayer->supportsPerspectiveTransform()) {
                    m_shouldAvoidPerspectiveTransform = true;
                    break;
                }
            }

            m_hasInvisibleNodes |= !node->visible(false);
        }
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

    QPointF originalCenterGeometric() const {
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

    ToolTransformArgs* currentConfig() const {
        return m_currentConfig;
    }

    KisNodeSP rootNode() const {
        return m_rootNode;
    }

    KisNodeList transformedNodes() const {
        return m_transformedNodes;
    }

    qreal basePreviewOpacity() const {
        return 0.9 * qreal(m_rootNode->opacity()) / 255.0;
    }

    bool shouldAvoidPerspectiveTransform() const {
        return m_shouldAvoidPerspectiveTransform;
    }

    bool hasInvisibleNodes() const {
        return m_hasInvisibleNodes;
    }

    void setCurrentConfigLocation(ToolTransformArgs *config) {
        m_currentConfig = config;
    }

private:
    /**
     * Information about the original selected rect
     * (before any transformations)
     */
    QRectF m_originalRect;
    ToolTransformArgs *m_currentConfig;
    KisNodeSP m_rootNode;
    KisNodeList m_transformedNodes;
    bool m_shouldAvoidPerspectiveTransform;
    bool m_hasInvisibleNodes;
};

#endif /* __TRANSFORM_TRANSACTION_PROPERTIES_H */
