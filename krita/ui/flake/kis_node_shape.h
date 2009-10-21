
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
#include <KoShape.h>
#include <KoViewConverter.h>

#include <krita_export.h>
#include <kis_types.h>

#define KIS_NODE_SHAPE_ID "KisNodeShape"

/**
   A KisNodeShape is a flake wrapper around adjustment nodes or paint
   nodes. A nodeshape can only have a KisMaskShape as its descendant.
 */
class KRITAUI_EXPORT KisNodeShape : public QObject, public KoShapeLayer
{
    Q_OBJECT
public:

    KisNodeShape(KoShapeContainer * parent, KisNodeSP node);
    virtual ~KisNodeShape();

    KisNodeSP node();

    // Shape overrides
    void paint(QPainter &painter, const KoViewConverter &converter);

    bool isSelectable() const {
        return false;
    }

    QSizeF size() const;

    QRectF boundingRect() const;

    void setPosition(const QPointF & position);

    // KoShapeContainer implementation
    void paintComponent(QPainter &painter, const KoViewConverter &converter);

    void addChild(KoShape * shape);

    /// reimplemented
    virtual void saveOdf(KoShapeSavingContext & context) const;

    // reimplemented
    virtual bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context);

private slots:

    void setNodeVisible(bool);

    void editabilityChanged();

    KisImageWSP getImage() const;

private:

    class Private;
    Private * const m_d;

};

#endif
