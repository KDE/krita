/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COORDINATES_CONVERTER_H
#define KIS_COORDINATES_CONVERTER_H

#include <QTransform>
#include <KoZoomHandler.h>

#include "kritaui_export.h"
#include "kis_types.h"

#define EPSILON 1e-6

#define SCALE_LESS_THAN(scX, scY, value)                        \
    (scX < (value) - EPSILON && scY < (value) - EPSILON)
#define SCALE_MORE_OR_EQUAL_TO(scX, scY, value)                 \
    (scX > (value) - EPSILON && scY > (value) - EPSILON)

namespace _Private
{
    template<class T> struct Traits
    {
        typedef T Result;
        static T map(const QTransform& transform, const T& obj)  { return transform.map(obj); }
    };

    template<> struct Traits<QRectF>
    {
        typedef QRectF Result;
        static QRectF map(const QTransform& transform, const QRectF& rc)  { return transform.mapRect(rc); }
    };

    template<> struct Traits<QRect>:    public Traits<QRectF>    { };
    template<> struct Traits<QPoint>:   public Traits<QPointF>   { };
    template<> struct Traits<QPolygon>: public Traits<QPolygonF> { };
    template<> struct Traits<QLine>:    public Traits<QLineF>    { };
}

class KRITAUI_EXPORT KisCoordinatesConverter: public KoZoomHandler
{
public:
    KisCoordinatesConverter();
    ~KisCoordinatesConverter() override;

    QSizeF getCanvasWidgetSize() const;

    void setCanvasWidgetSize(QSizeF size);
    void setDevicePixelRatio(qreal value);
    void setImage(KisImageWSP image);
    void setDocumentOffset(const QPointF &offset);

    qreal devicePixelRatio() const;
    QPoint documentOffset() const;
    qreal rotationAngle() const;

    QPoint rotate(QPointF center, qreal angle);
    QPoint mirror(QPointF center, bool mirrorXAxis, bool mirrorYAxis);
    bool xAxisMirrored() const;
    bool yAxisMirrored() const;
    QPoint resetRotation(QPointF center);

    void setZoom(qreal zoom) override;

    /**
     * A composition of to scale methods: zoom level + image resolution
     */
    qreal effectiveZoom() const;

    template<class T> typename _Private::Traits<T>::Result
    imageToViewport(const T& obj) const { return _Private::Traits<T>::map(imageToViewportTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    viewportToImage(const T& obj) const { return _Private::Traits<T>::map(imageToViewportTransform().inverted(), obj); }

    template<class T> typename _Private::Traits<T>::Result
    flakeToWidget(const T& obj) const { return _Private::Traits<T>::map(flakeToWidgetTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    widgetToFlake(const T& obj) const { return _Private::Traits<T>::map(flakeToWidgetTransform().inverted(), obj); }

    template<class T> typename _Private::Traits<T>::Result
    widgetToViewport(const T& obj) const { return _Private::Traits<T>::map(viewportToWidgetTransform().inverted(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    viewportToWidget(const T& obj) const { return _Private::Traits<T>::map(viewportToWidgetTransform(), obj); }

    template<class T> typename _Private::Traits<T>::Result
    documentToWidget(const T& obj) const { return _Private::Traits<T>::map(documentToWidgetTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    widgetToDocument(const T& obj) const { return _Private::Traits<T>::map(documentToWidgetTransform().inverted(), obj); }

    template<class T> typename _Private::Traits<T>::Result
    imageToDocument(const T& obj) const { return _Private::Traits<T>::map(imageToDocumentTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    documentToImage(const T& obj) const { return _Private::Traits<T>::map(imageToDocumentTransform().inverted(), obj); }

    template<class T> typename _Private::Traits<T>::Result
    documentToFlake(const T& obj) const { return _Private::Traits<T>::map(documentToFlakeTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    flakeToDocument(const T& obj) const { return _Private::Traits<T>::map(documentToFlakeTransform().inverted(), obj); }

    template<class T> typename _Private::Traits<T>::Result
    imageToWidget(const T& obj) const { return _Private::Traits<T>::map(imageToWidgetTransform(), obj); }
    template<class T> typename _Private::Traits<T>::Result
    widgetToImage(const T& obj) const { return _Private::Traits<T>::map(imageToWidgetTransform().inverted(), obj); }

    QTransform imageToWidgetTransform() const;
    QTransform imageToDocumentTransform() const;
    QTransform documentToFlakeTransform() const;
    QTransform imageToViewportTransform() const;
    QTransform viewportToWidgetTransform() const;
    QTransform flakeToWidgetTransform() const;
    QTransform documentToWidgetTransform() const;

    void getQPainterCheckersInfo(QTransform *transform,
                                 QPointF *brushOrigin,
                                 QPolygonF *poligon,
                                 const bool scrollCheckers) const;

    void getOpenGLCheckersInfo(const QRectF &viewportRect,
                               QTransform *textureTransform,
                               QTransform *modelTransform,
                               QRectF *textureRect,
                               QRectF *modelRect,
                               const bool scrollCheckers) const;

    QPointF imageCenterInWidgetPixel() const;
    QRectF imageRectInWidgetPixels() const;
    QRectF imageRectInViewportPixels() const;
    QSizeF imageSizeInFlakePixels() const;
    QRectF widgetRectInFlakePixels() const;
    QRectF widgetRectInImagePixels() const;
    QRect imageRectInImagePixels() const;
    QRectF imageRectInDocumentPixels() const;

    QPointF flakeCenterPoint() const;
    QPointF widgetCenterPoint() const;

    void imageScale(qreal *scaleX, qreal *scaleY) const;
    void imagePhysicalScale(qreal *scaleX, qreal *scaleY) const;

    QPointF snapToDevicePixel(const QPointF &point) const;

public:
    // overrides from KoViewConverter
    QTransform viewToWidget() const override;
    QTransform widgetToView() const override;

private:
    friend class KisZoomAndPanTest;

    QPointF centeringCorrection() const;
    void correctOffsetToTransformation();
    void correctTransformationToOffset();
    void recalculateTransformations();

private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_COORDINATES_CONVERTER_H */
