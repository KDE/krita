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

#include "krita_export.h"
#include "kis_types.h"


class QSize;
class QSizeF;
class QRectF;
class QPoint;
class QPolygonF;
class QTransform;
class KoViewConverter;


/**
 * Automatic generation of QRectF and QPointF transformation methods.
 * They will use pre-generated QTransform objects to perform the change.
 */
#define DEFINE_RECT_METHOD(name,transform)                              \
    QRectF KisCoordinatesConverter::name(const QRectF &rc) const        \
    {                                                                   \
        return (transform).mapRect(rc);                                 \
    }

#define DEFINE_POINT_METHOD(name,transform)                             \
    QPointF KisCoordinatesConverter::name(const QPointF &pt) const      \
    {                                                                   \
        return (transform).map(pt);                                     \
    }

#define DECLARE_RECT_METHOD(name)               \
    QRectF name(const QRectF &rc) const

#define DECLARE_POINT_METHOD(name)              \
    QPointF name(const QPointF &rc) const


#define DEFINE_TRANSFORM_METHODS(name,invertedName,transform)         \
    DEFINE_RECT_METHOD(name, transform);                              \
    DEFINE_RECT_METHOD(invertedName, (transform).inverted());         \
    DEFINE_POINT_METHOD(name, transform);                             \
    DEFINE_POINT_METHOD(invertedName, (transform).inverted())         \

#define DECLARE_TRANSFORM_METHODS(name,invertedName)                   \
    DECLARE_RECT_METHOD(name);                                         \
    DECLARE_RECT_METHOD(invertedName);                                 \
    DECLARE_POINT_METHOD(name);                                        \
    DECLARE_POINT_METHOD(invertedName)                                 \




class KRITAUI_EXPORT KisCoordinatesConverter
{
public:
    KisCoordinatesConverter(KoViewConverter *viewConverter);
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


    // Automatic methods generation. See a comment above.
    DECLARE_TRANSFORM_METHODS(imageToViewport, viewportToImage);
    DECLARE_TRANSFORM_METHODS(widgetToViewport, viewportToWidget);
    DECLARE_TRANSFORM_METHODS(documentToWidget, widgetToDocument);
    DECLARE_TRANSFORM_METHODS(imageToDocument, documentToImage);


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
                               QRectF *modelRect);

    QRectF imageRectInWidgetPixels() const;
    QRectF imageRectInViewportPixels() const;
    QSizeF imageSizeInFlakePixels() const;
    QRectF widgetRectInFlakePixels() const;

    QPoint offsetFromFlakeCenterPoint(const QPointF &pt) const;

    void imageScale(qreal *scaleX, qreal *scaleY) const;

private:
    void recalculateTransformations() const;

private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_COORDINATES_CONVERTER_H */
