/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include <QTransform>
#include <KoZoomHandler.h>

#include "krita_export.h"
#include "kis_types.h"

class QSize;
class QSizeF;
class QRectF;
class QPoint;

namespace _Private
{
    template<class T> struct Traits { };

    template<> struct Traits<QRectF>
    {
        typedef QRectF Result;
        static QRectF map(const QTransform& transform, const QRectF& rc)  { return transform.mapRect(rc); }
    };
    
    template<> struct Traits<QRect>: public Traits<QRectF> { };

    template<> struct Traits<QPointF>
    {
        typedef QPointF Result;
        static QPointF map(const QTransform& transform, const QPointF& pt)  { return transform.map(pt); }
    };

    template<> struct Traits<QPoint>: public Traits<QPointF> { };
}

class KRITAUI_EXPORT KisCoordinatesConverter: public KoZoomHandler
{
public:
    KisCoordinatesConverter();
    ~KisCoordinatesConverter();

    void setCanvasWidgetSize(QSize size);
    void setImage(KisImageWSP image);
    void setDocumentOrigin(const QPoint &origin);
    void setDocumentOffset(const QPoint &offset);

    QPoint documentOrigin() const;
    QPoint documentOffset() const;
    
    void rotate(qreal angle);
    void mirror(bool mirrorXAxis, bool mirrorYAxis);
    void resetTransformations();
    
    virtual void setZoom(qreal zoom);
    
    template<class T> typename _Private::Traits<T>::Result
    imageToViewport(const T& obj) const { return _Private::Traits<T>::map(imageToViewportTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    viewportToImage(const T& obj) const { return _Private::Traits<T>::map(imageToViewportTransform().inverted(), obj); }
    
    template<class T> typename _Private::Traits<T>::Result
    flakeToWidget(const T& obj) const { return _Private::Traits<T>::map(flakeToWidgetTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    widgetToFlake(const T& obj) const { return _Private::Traits<T>::map(flakeToWidgetTransform().inverted(), obj); }
    
    template<class T> typename _Private::Traits<T>::Result
    widgetToViewport(const T& obj) const { return _Private::Traits<T>::map(widgetToViewportTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    viewportToWidget(const T& obj) const { return _Private::Traits<T>::map(widgetToViewportTransform().inverted(), obj); }
    
    template<class T> typename _Private::Traits<T>::Result
    documentToWidget(const T& obj) const { return _Private::Traits<T>::map(documentToWidgetTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    widgetToDocument(const T& obj) const { return _Private::Traits<T>::map(documentToWidgetTransform().inverted(), obj); }
    
    template<class T> typename _Private::Traits<T>::Result
    imageToDocument(const T& obj) const { return _Private::Traits<T>::map(imageToDocumentTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    documentToImage(const T& obj) const { return _Private::Traits<T>::map(imageToDocumentTransform().inverted(), obj); }
    
    template<class T> typename _Private::Traits<T>::Result
    imageToWidget(const T& obj) const { return _Private::Traits<T>::map(imageToWidgetTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    widgetToImage(const T& obj) const { return _Private::Traits<T>::map(imageToWidgetTransform().inverted(), obj); }

    QTransform imageToWidgetTransform() const;
    QTransform imageToDocumentTransform() const;
    QTransform imageToViewportTransform() const;
    QTransform widgetToViewportTransform() const;
    QTransform flakeToWidgetTransform() const;
    QTransform documentToWidgetTransform() const;

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
