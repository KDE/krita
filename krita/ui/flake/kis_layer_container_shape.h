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

#ifndef KIS_LAYER_CONTAINER_SHAPE_H_
#define KIS_LAYER_CONTAINER_SHAPE_H_

#include <QObject>

#include <QString>

#include <KoShapeLayer.h>
#include <kis_types.h>
#include <krita_export.h>

class QPainter;
class KoViewConverter;


const QString KIS_LAYER_CONTAINER_ID = "KisLayerContainerShapeID";
/**
   The layer container is the flake shape that corresponds to
   KisGroupLayer. It contains any number of layers, including other
   group layers.
 */
class KRITAUI_EXPORT KisLayerContainerShape : public QObject, public KoShapeLayer
{
    Q_OBJECT
public:

    KisLayerContainerShape(KoShapeContainer * parent, KisLayerSP groupLayer);

    virtual ~KisLayerContainerShape();

public:

    KisLayerSP groupLayer();

    // KoShapeContainer implementation
    void paintComponent(QPainter &painter, const KoViewConverter &converter);

    // KoShape overrides
    bool isSelectable() const {
        return false;
    }

    QSizeF size() const;
    QRectF boundingRect() const;

    /// reimplemented
    virtual void saveOdf(KoShapeSavingContext & context) const;
    // reimplemented
    virtual bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context);
private slots:
    void setLayerVisible(bool);
    void editabilityChanged();
private:
    class Private;
    Private * const m_d;
};

#endif
