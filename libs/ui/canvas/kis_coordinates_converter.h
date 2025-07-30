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

class KoViewTransformStillPoint;

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
    QSize viewportDevicePixelSize() const;

    void setCanvasWidgetSize(QSizeF size);
    void setDevicePixelRatio(qreal value);
    void setImage(KisImageWSP image);
    void setExtraReferencesBounds(const QRect &imageRect);
    void setImageBounds(const QRect &rect, const QPointF oldImageStillPoint, const QPointF newImageStillPoint);
    void setImageResolution(qreal xRes, qreal yRes);
    void setDocumentOffset(const QPointF &offset);

    qreal devicePixelRatio() const;
    QPoint documentOffset() const;
    QPointF documentOffsetF() const;
    qreal rotationAngle() const;


    /**
     * \brief returns the point in image coordinates which is supposed to be
     *        the default still point for the transformations of the canvas.
     *        It returns the point of the image that is "roughly" mapped to
     *        to the center of the canvas widget.
     *
     * Some of the methods of the converter accept std::optional<KoViewTransformStillPoint>
     * for a still point, over which the transformation should happen. When this argument is
     * std::nullotp, then preferredTransformationCenter() is used.
     *
     * One important property of preferredTransformationCenter() is that it is **not**
     * changed when the canvas is transformed over it, even when pixel alignment happens.
     * It means that preferredTransformationCenter() may **not** exactly map to the center
     * of the widget, due to hardware-pixel-alignment.
     *
     * Keeping this value unchanged allows us to avoid drifts of the offset when zooming
     * and rotating the canvas.
     */
    QPointF preferredTransformationCenter() const;

    // Use the begin/end interface to rotate the canvas in one transformation.
    // This method is more accurate and doesn't amplify numerical errors from very small angles.
    void beginRotation();
    void endRotation();

    void enableNatureGestureFlag();

    /**
     * \brief rotates the canvas
     *
     * For the meaning of \p stillPoint \see setZoom()
     */
    void rotate(const std::optional<KoViewTransformStillPoint> &stillPoint, qreal angle);

    /**
     * \brief mirrors the canvas
     *
     * For the meaning of \p stillPoint \see setZoom()
     */
    void mirror(const std::optional<KoViewTransformStillPoint> &stillPoint, bool mirrorXAxis, bool mirrorYAxis);

    bool xAxisMirrored() const;
    bool yAxisMirrored() const;

    /**
     * \brief resets canvas rotation
     *
     * For the meaning of \p stillPoint \see setZoom()
     */
    void resetRotation(const std::optional<KoViewTransformStillPoint> &stillPoint);

    void setZoom(qreal zoom) override;

    void zoomTo(const QRectF &widgetRect);

    /**
     * \brief changes the zoom mode of the canvas
     *
     * When \p mode is KoZoomMode::ZOOM_CONSTANT, \p stillPoint instructs the converter
     * to keep the passed point "still", i.e. to make sure that stillPoint.docPoint()
     * maps to stillPoint.widgetPoint() on screen.
     *
     * Please make sure that there is **no guarantee** that the still point will map
     * to the passed points exactly! The still point may be offset by at most
     * (0.5 * sqrt(2) / devicePixelRatio()) due to hardware pixel alignment.
     *
     * This alignement is the reason why we pass both document and view points
     * as a still point, because both values should be kept constant during iterative
     * zoom operations. Otherwise the canvas will drift to the side because of the
     * pixel alignmentl.
     *
     * If \p stillPoint is std::nullopt, then the zooming happens over
     * preferredTransformationCenter(), which is basically the center of
     * canvas widget, but with some guards against the drifting.
     */
    void setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const std::optional<KoViewTransformStillPoint> &stillPoint);

    void setCanvasWidgetSizeKeepZoom(const QSizeF &size);

    /**
     * A composition of to scale methods: zoom level + image resolution
     */
    qreal effectiveZoom() const;
    qreal effectivePhysicalZoom() const;

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
                                 QPolygonF *polygon,
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
    QSizeF snapWidgetSizeToDevicePixel(const QSizeF &size) const;

    QPoint minimumOffset() const;
    QPoint maximumOffset() const;

    qreal minZoom() const;
    qreal maxZoom() const;
    qreal clampZoom(qreal zoom) const;
    QVector<qreal> standardZoomLevels() const;

    static qreal findNextZoom(qreal currentZoom, const QVector<qreal> &zoomLevels);
    static qreal findPrevZoom(qreal currentZoom, const QVector<qreal> &zoomLevels);

    /**
     * \brief Creates a still point that links the \p viewPoint of the widget
     *        to the corresponding point of the image.
     *
     * The link is "baked" in KoViewTransformStillPoint object, hence
     * intermediate transformations will not affect it.
     */
    KoViewTransformStillPoint makeViewStillPoint(const QPointF &viewPoint) const;

    /**
     * \brief Creates a still point that links the \p docPoint of the image
     *        (in document pixels!) to the curresponding point on the screen
     *        (in the canvas widget).
     *
     * The link is "baked" in KoViewTransformStillPoint object, hence
     * intermediate transformations will not affect it.
     */
    KoViewTransformStillPoint makeDocStillPoint(const QPointF &docPoint) const;

public:
    // overrides from KoViewConverter
    QTransform viewToWidget() const override;
    QTransform widgetToView() const override;

private:
    friend class KisZoomAndPanTest;
    friend class KisCoordinatesConverterTest;

    QPointF centeringCorrection() const;
    void correctOffsetToTransformationAndSnap();
    void correctTransformationToOffset();
    void resetPreferredTransformationCenter();
    void recalculateTransformations();
    void recalculateZoomLevelLimits();
    void recalculateOffsetBoundsAndCrop();

private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_COORDINATES_CONVERTER_H */
