/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_NODE_SHAPE_H_
#define KIS_NODE_SHAPE_H_

#include <QObject>

#include <KoShapeLayer.h>

#include <kritaui_export.h>
#include <kis_types.h>


#define KIS_NODE_SHAPE_ID "KisNodeShape"

/**
 * A KisNodeShape is a flake wrapper around Krita nodes. It is used
 * for dealing with currently active node for tools.
 */
class KRITAUI_EXPORT KisNodeShape : public QObject, public KoShapeLayer
{
    Q_OBJECT
public:
    KisNodeShape(KisNodeSP node);
    ~KisNodeShape() override;

    KisNodeSP node();

    // Empty implementations as the node is not painted anywhere
    QSizeF size() const override;
    QRectF boundingRect() const override;
    void setPosition(const QPointF &) override;
    void paint(QPainter &painter, KoShapePaintingContext &paintcontext) const override;

private Q_SLOTS:
    void editabilityChanged();

private:
    bool checkIfDescendant(KoShapeLayer *activeLayer);

private:
    struct Private;
    Private * const m_d;
};

#endif
