/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TRANSFORM_TRANSACTION_PROPERTIES_H
#define __TRANSFORM_TRANSACTION_PROPERTIES_H

#include <QRectF>
#include <QPointF>
#include "kis_node.h"
#include "kis_layer_utils.h"
#include "kis_external_layer_iface.h"
#include "kis_transform_mask.h"

class ToolTransformArgs;

class TransformTransactionProperties
{
public:
    TransformTransactionProperties()
    {
    }

TransformTransactionProperties(const QRectF &originalRect,
                               ToolTransformArgs *currentConfig,
                               KisNodeList rootNodes,
                               const QList<KisNodeSP> &transformedNodes)
        : m_originalRect(originalRect),
          m_currentConfig(currentConfig),
          m_rootNodes(rootNodes),
          m_transformedNodes(transformedNodes),
          m_shouldAvoidPerspectiveTransform(false),
          m_boundsRotationAllowed(true)
    {
        m_hasInvisibleNodes = false;
        Q_FOREACH (KisNodeSP node, transformedNodes) {
            if (KisExternalLayer *extLayer = dynamic_cast<KisExternalLayer*>(node.data())) {
                if (!extLayer->supportsPerspectiveTransform()) {
                    m_shouldAvoidPerspectiveTransform = true;
                    break;
                }
            }
            if (dynamic_cast<const KisTransformMask*>(node.data())) {
                m_boundsRotationAllowed = false;
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

    KisNodeList rootNodes() const {
        return m_rootNodes;
    }

    KisNodeList transformedNodes() const {
        return m_transformedNodes;
    }

    qreal basePreviewOpacity() const {
        // Todo: this doesn't work for multiple layers
        return 0.9 * qreal(m_rootNodes[0]->opacity()) / 255.0;
    }

    const QPolygon &convexHull() {
        return m_convexHull;
    }

    bool shouldAvoidPerspectiveTransform() const {
        return m_shouldAvoidPerspectiveTransform;
    }

    bool hasInvisibleNodes() const {
        return m_hasInvisibleNodes;
    }

    bool boundsRotationAllowed() const {
        return m_boundsRotationAllowed;
    }

    void setCurrentConfigLocation(ToolTransformArgs *config) {
        m_currentConfig = config;
    }

    void setConvexHull(QPolygon hull) {
        m_convexHull = std::move(hull);
    }

private:
    /**
     * Information about the original selected rect
     * (before any transformations)
     */
    QRectF m_originalRect;
    ToolTransformArgs *m_currentConfig {0};
    KisNodeList m_rootNodes;
    KisNodeList m_transformedNodes;
    QPolygon m_convexHull;
    bool m_shouldAvoidPerspectiveTransform {false};
    bool m_hasInvisibleNodes {false};
    bool m_boundsRotationAllowed {true};
};

#endif /* __TRANSFORM_TRANSACTION_PROPERTIES_H */
