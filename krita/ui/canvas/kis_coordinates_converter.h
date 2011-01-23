/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_COORDINATES_CONVERTER_H
#define KIS_COORDINATES_CONVERTER_H

#include <KoZoomHandler.h>

#include "krita_export.h"
#include "kis_types.h"

class QSize;
class QSizeF;
class QRectF;
class QPoint;
class QPolygonF;
class QTransform;
class KoViewConverter;

class KRITAUI_EXPORT KisCoordinatesConverter: public KoZoomHandler
{
public:
    KisCoordinatesConverter();
    ~KisCoordinatesConverter();

    void notifyZoomChanged();
    void setCanvasWidgetSize(QSize size);
    void setImage(KisImageWSP image);
    void setDocumentOrigin(const QPoint &origin);
    void setDocumentOffset(const QPoint &offset);

    QPoint documentOrigin() const;
    QPoint documentOffset() const;

    void setPostprocessingTransform(const QTransform &transform);
    QTransform postprocessingTransform() const;

    QRectF  imageToViewport(const QRectF &rc) const;
    QRectF  viewportToImage(const QRectF &rc) const;
    QPointF imageToViewport(const QPointF &rc) const;
    QPointF viewportToImage(const QPointF &rc) const;
    
    QRectF  flakeToWidget(const QRectF &rc) const;
    QRectF  widgetToFlake(const QRectF &rc) const;
    QPointF flakeToWidget(const QPointF &rc) const;
    QPointF widgetToFlake(const QPointF &rc) const;
    
    QRectF  widgetToViewport(const QRectF &rc) const;
    QRectF  viewportToWidget(const QRectF &rc) const;
    QPointF widgetToViewport(const QPointF &rc) const;
    QPointF viewportToWidget(const QPointF &rc) const;
    
    QRectF  documentToWidget(const QRectF &rc) const;
    QRectF  widgetToDocument(const QRectF &rc) const;
    QPointF documentToWidget(const QPointF &rc) const;
    QPointF widgetToDocument(const QPointF &rc) const;
    
    QRectF  imageToDocument(const QRectF &rc) const;
    QRectF  documentToImage(const QRectF &rc) const;
    QPointF imageToDocument(const QPointF &rc) const;
    QPointF documentToImage(const QPointF &rc) const;

    QTransform imageToWidgetTransform() const;
    QTransform documentToWidgetTransform() const;
    QTransform flakeToWidgetTransform() const;
    QTransform viewportToWidgetTransform() const;

    void getQPainterCheckersInfo(QTransform *transform,
                                 QPointF *brushOrigin,
                                 QPolygonF *poligon) const;

    void getOpenGLCheckersInfo(QTransform *textureTransform,
                               QTransform *modelTransform,
                               QRectF *textureRect,
                               QRectF *modelRect) const;

    QRectF imageRectInWidgetPixels() const;
    QRectF imageRectInViewportPixels() const;
    QSizeF imageSizeInFlakePixels() const;
    QRectF widgetRectInFlakePixels() const;

    QPoint shiftFromFlakeCenterPoint(const QPointF &pt) const;
    QPointF flakeCenterPoint() const;

    void imageScale(qreal *scaleX, qreal *scaleY) const;

private:
    void recalculateTransformations() const;

private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_COORDINATES_CONVERTER_H */
