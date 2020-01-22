/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
    void saveOdf(KoShapeSavingContext & context) const override;
    bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context) override;

private Q_SLOTS:
    void editabilityChanged();

private:
    bool checkIfDescendant(KoShapeLayer *activeLayer);

private:
    struct Private;
    Private * const m_d;
};

#endif
