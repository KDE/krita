/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include "kis_coordinates_converter.h"

#include <QtMath>
#include <QTransform>
#include <KoViewConverter.h>

#include <kis_config.h>
#include <kis_image.h>
#include <kis_algebra_2d.h>


struct KisCoordinatesConverter::Private {
    Private():
        imageXRes(1.0),
        imageYRes(1.0),
        isXAxisMirrored(false),
        isYAxisMirrored(false),
        isRotating(false),
        isNativeGesture(false),
        rotationAngle(0.0),
        rotationBaseAngle(0.0),
        devicePixelRatio(1.0)
    {
    }

    QRect imageBounds;
    qreal imageXRes;
    qreal imageYRes;

    bool isXAxisMirrored;
    bool isYAxisMirrored;
    bool isRotating;
    bool isNativeGesture;
    qreal rotationAngle;
    qreal rotationBaseAngle;
    QSizeF canvasWidgetSize;
    qreal devicePixelRatio;
    QPointF documentOffset;
    QPoint minimumOffset;
    QPoint maximumOffset;

    QTransform flakeToWidget;
    QTransform rotationBaseTransform;
    QTransform imageToDocument;
    QTransform documentToFlake;
    QTransform widgetToViewport;
};

/**
 * When vastScrolling value is less than 0.5 it is possible
 * that the whole scrolling area (viewport) will be smaller than
 * the size of the widget. In such cases the image should be
 * centered in the widget. Previously we used a special parameter
 * documentOrigin for this purpose, now the value for this
 * centering is calculated dynamically, helping the offset to
 * center the image inside the widget
 *
 * Note that the correction is null when the size of the document
 * plus vast scrolling reserve is larger than the widget. This
 * is always true for vastScrolling parameter > 0.5.
 */

QPointF KisCoordinatesConverter::centeringCorrection() const
{
    KisConfig cfg(true);

    QSize documentSize = imageRectInWidgetPixels().toAlignedRect().size();
    QPointF dPoint(documentSize.width(), documentSize.height());
    QPointF wPoint(m_d->canvasWidgetSize.width(), m_d->canvasWidgetSize.height());

    QPointF minOffset = -cfg.vastScrolling() * wPoint;
    QPointF maxOffset = dPoint - wPoint + cfg.vastScrolling() * wPoint;

    QPointF range = maxOffset - minOffset;

    range.rx() = qMin(range.x(), (qreal)0.0);
    range.ry() = qMin(range.y(), (qreal)0.0);

    range /= 2;

    return -range;
}

void KisCoordinatesConverter::recalculateOffsetBoundsAndCrop()
{
    if (!m_d->canvasWidgetSize.isValid()) return;

    KisConfig cfg(true);

    // TODO: take references into account
    QSize documentSize = imageRectInWidgetPixels().toAlignedRect().size();
    QPointF dPoint(documentSize.width(), documentSize.height());
    QPointF wPoint(m_d->canvasWidgetSize.width(), m_d->canvasWidgetSize.height());

    QPointF minOffset = -cfg.vastScrolling() * wPoint;
    QPointF maxOffset = dPoint - wPoint + cfg.vastScrolling() * wPoint;

    m_d->minimumOffset = minOffset.toPoint();
    m_d->maximumOffset = maxOffset.toPoint();

    const QRectF limitRect(m_d->minimumOffset, m_d->maximumOffset);

    if (!limitRect.contains(m_d->documentOffset)) {
        m_d->documentOffset = KisAlgebra2D::clampPoint(m_d->documentOffset, limitRect);
        qDebug() << "    corrected offset:" << m_d->documentOffset;
        correctTransformationToOffset();
    }
}

QPoint KisCoordinatesConverter::minimumOffset() const
{
    return m_d->minimumOffset;
}

QPoint KisCoordinatesConverter::maximumOffset() const
{
    return m_d->maximumOffset;
}

/**
 * The document offset and the position of the top left corner of the
 * image must always coincide, that is why we need to correct them to
 * and fro.
 *
 * When we change zoom level, the calculation of the new offset is
 * done by KoCanvasControllerWidget, that is why we just passively fix
 * the flakeToWidget transform to conform the offset and wait until
 * the canvas controller will recenter us.
 *
 * But when we do our own transformations of the canvas, like rotation
 * and mirroring, we cannot rely on the centering of the canvas
 * controller and we do it ourselves. Then we just set new offset and
 * return its value to be set in the canvas controller explicitly.
 */

void KisCoordinatesConverter::correctOffsetToTransformation()
{
    m_d->documentOffset = snapToDevicePixel(-(imageRectInWidgetPixels().topLeft() -
          centeringCorrection()));
}

void KisCoordinatesConverter::correctTransformationToOffset()
{
    QPointF topLeft = imageRectInWidgetPixels().topLeft();
    QPointF diff = (-topLeft) - m_d->documentOffset;
    diff += centeringCorrection();
    m_d->flakeToWidget *= QTransform::fromTranslate(diff.x(), diff.y());
}

void KisCoordinatesConverter::recalculateTransformations()
{
    m_d->imageToDocument = QTransform::fromScale(1 / m_d->imageXRes,
                                                 1 / m_d->imageYRes);

    qreal zoomX, zoomY;
    KoZoomHandler::zoom(&zoomX, &zoomY);
    m_d->documentToFlake = QTransform::fromScale(zoomX, zoomY);

    correctTransformationToOffset();
    recalculateOffsetBoundsAndCrop();

    QRectF irect = imageRectInWidgetPixels();
    QRectF wrect = QRectF(QPoint(0,0), m_d->canvasWidgetSize);
    QRectF rrect = irect & wrect;

    QTransform reversedTransform = flakeToWidgetTransform().inverted();
    QRectF     canvasBounds      = reversedTransform.mapRect(rrect);
    QPointF    offset            = canvasBounds.topLeft();

    m_d->widgetToViewport = reversedTransform * QTransform::fromTranslate(-offset.x(), -offset.y());
}


KisCoordinatesConverter::KisCoordinatesConverter()
    : m_d(new Private) { }

KisCoordinatesConverter::~KisCoordinatesConverter()
{
    delete m_d;
}

QSizeF KisCoordinatesConverter::getCanvasWidgetSize() const
{
    return m_d->canvasWidgetSize;
}

void KisCoordinatesConverter::setCanvasWidgetSize(QSizeF size)
{
    m_d->canvasWidgetSize = size;
    recalculateTransformations();
}

void KisCoordinatesConverter::setDevicePixelRatio(qreal value)
{
    m_d->devicePixelRatio = value;
}

void KisCoordinatesConverter::setImage(KisImageWSP image)
{
    m_d->imageXRes = image->xRes();
    m_d->imageYRes = image->yRes();

    // we should **not** call setResolution() here, since
    // it is a different kind of resolution that is used
    // to convert the image to the physical size of the display

    m_d->imageBounds = image->bounds();
    recalculateTransformations();
}

void KisCoordinatesConverter::setImageBounds(const QRect &rect)
{
    if (rect == m_d->imageBounds) return;
    m_d->imageBounds = rect;
    recalculateTransformations();
}

void KisCoordinatesConverter::setImageResolution(qreal xRes, qreal yRes)
{
    // we consiter the center of the image to be the still point
    // on the canvas

    if (qFuzzyCompare(xRes, m_d->imageXRes) && qFuzzyCompare(yRes, m_d->imageYRes)) return;

    const QPointF oldImageCenter = imageCenterInWidgetPixel();

    // we should **not** call setResolution() here, since
    // it is a different kind of resolution that is used
    // to convert the image to the physical size of the display

    m_d->imageXRes = xRes;
    m_d->imageYRes = yRes;
    recalculateTransformations();

    const QPointF newImageCenter = imageCenterInWidgetPixel();
    m_d->documentOffset += newImageCenter - oldImageCenter;
    recalculateTransformations();
}

void KisCoordinatesConverter::setDocumentOffset(const QPointF& offset)
{
    QPointF diff = m_d->documentOffset - offset;

    m_d->documentOffset = offset;
    m_d->flakeToWidget *= QTransform::fromTranslate(diff.x(), diff.y());
    recalculateTransformations();
}

qreal KisCoordinatesConverter::devicePixelRatio() const
{
    return m_d->devicePixelRatio;
}

QPoint KisCoordinatesConverter::documentOffset() const
{
    return QPoint(int(m_d->documentOffset.x()), int(m_d->documentOffset.y()));
}

qreal KisCoordinatesConverter::rotationAngle() const
{
    return m_d->rotationAngle;
}

void KisCoordinatesConverter::setZoom(qreal zoom)
{
    KoZoomHandler::setZoom(zoom);
    recalculateTransformations();
}

void KisCoordinatesConverter::setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const QPointF &stillPoint)
{
    if (this->zoomMode() == mode &&
            qFuzzyCompare(this->zoom(), zoom) &&
            qFuzzyCompare(m_d->imageXRes, resolutionX) &&
            qFuzzyCompare(m_d->imageYRes, resolutionY)) {
        return; // no change
    }

    const int cfgMargin = zoomMarginSize();

    auto updateDisplayResolution = [&]() {
        if (!qFuzzyCompare(resolutionX, this->resolutionX()) || !qFuzzyCompare(resolutionY, this->resolutionY())) {
            setResolution(resolutionX, resolutionY);
        }
    };

    if(mode == KoZoomMode::ZOOM_CONSTANT) {
        if(qFuzzyIsNull(zoom)) return;
        const QPointF oldStillPointInImagePixels =
            widgetToImage(stillPoint);

        updateDisplayResolution();
        KoZoomHandler::setZoom(zoom);
        KoZoomHandler::setZoomMode(mode);
        recalculateTransformations();

        const QPointF newStillPoint = imageToWidget(oldStillPointInImagePixels);
        const QPointF offset = newStillPoint - stillPoint;
        setDocumentOffset(m_d->documentOffset + offset);
    } else if (mode == KoZoomMode::ZOOM_WIDTH || mode == KoZoomMode::ZOOM_HEIGHT) {
        // clang-format off
        struct DimensionWrapperHorizontal {
            qreal primaryLength(const QRectF &obj) { return obj.width(); }
            qreal primaryLength(const QSizeF &obj) { return obj.width(); }
            qreal primaryStart(const QRectF &obj) { return obj.left(); }
            qreal primaryEnd(const QRectF &obj) { return obj.right(); }
            qreal secondaryStart(const QRectF &obj) { return obj.top(); }
            qreal secondaryEnd(const QRectF &obj) { return obj.bottom(); }
            qreal& primaryRef(QPointF &pt) { return pt.rx(); }
            qreal& secondaryRef(QPointF &pt) { return pt.ry(); }
            qreal primary(const QPointF &pt) { return pt.x(); }
            qreal secondary(const QPointF &pt) { return pt.y(); }
        };

        struct DimensionWrapperVertical {
            qreal primaryLength(const QRectF &obj) { return obj.height(); }
            qreal primaryLength(const QSizeF &obj) { return obj.height(); }
            qreal primaryStart(const QRectF &obj) { return obj.top(); }
            qreal primaryEnd(const QRectF &obj) { return obj.bottom(); }
            qreal secondaryStart(const QRectF &obj) { return obj.left(); }
            qreal secondaryEnd(const QRectF &obj) { return obj.right(); }
            qreal& primaryRef(QPointF &pt) { return pt.ry(); }
            qreal& secondaryRef(QPointF &pt) { return pt.rx(); }
            qreal primary(const QPointF &pt) { return pt.y(); }
            qreal secondary(const QPointF &pt) { return pt.x(); }
        };
        // clang-format on

        auto zoomToDimension = [&](auto dim) {
            /**
             * We try not to move the image alond the secondary axis, we cleverly choose
             * a still point and pin it along the transformation
             */
            QPointF stillPointInImagePixels;
            QPointF stillPointInOldWidgetPixels;

            {
                /**
                 * Depending on whether the image covers the center of the widget,
                 * we either scale relative to the center of the widget or the center
                 * of the image.
                 */
                const QPointF widgetCenterInImagePixels = widgetToImage(widgetCenterPoint());

                if (dim.secondary(widgetCenterInImagePixels) >= dim.secondaryStart(imageRectInImagePixels())
                    && dim.secondary(widgetCenterInImagePixels) <= dim.secondaryEnd(imageRectInImagePixels())) {
                    stillPointInImagePixels = widgetCenterInImagePixels;
                    stillPointInOldWidgetPixels = widgetCenterPoint();
                } else {
                    stillPointInImagePixels = QRectF(imageRectInImagePixels()).center();
                    stillPointInOldWidgetPixels = imageToWidget(stillPointInImagePixels);
                }
            }

            updateDisplayResolution();
            recalculateTransformations();

            const QSize documentSize = imageRectInWidgetPixels().toAlignedRect().size();
            const qreal zoomCoeff =
                (dim.primaryLength(m_d->canvasWidgetSize) - 2 * cfgMargin) / dim.primaryLength(documentSize);

            KoZoomHandler::setZoom(this->zoom() * zoomCoeff);
            KoZoomHandler::setZoomMode(mode);
            recalculateTransformations();

            const QPointF stillPointInNewWidgetPixels = imageToWidget(stillPointInImagePixels);
            const qreal verticalOffset =
                -dim.secondary(stillPointInOldWidgetPixels) + dim.secondary(stillPointInNewWidgetPixels);
            QPointF newDocumentOffset;
            dim.primaryRef(newDocumentOffset) = -cfgMargin;
            dim.secondaryRef(newDocumentOffset) = dim.secondary(m_d->documentOffset) + verticalOffset;

            setDocumentOffset(newDocumentOffset);
        };

        if (mode == KoZoomMode::ZOOM_WIDTH) {
            zoomToDimension(DimensionWrapperHorizontal{});
        } else {
            zoomToDimension(DimensionWrapperVertical{});
        }

    } else if (mode == KoZoomMode::ZOOM_PAGE) {
        updateDisplayResolution();
        recalculateTransformations();

        const QSize documentSize = imageRectInWidgetPixels().toAlignedRect().size();
        const qreal zoomCoeffX = (m_d->canvasWidgetSize.width() - 2 * cfgMargin) / documentSize.width();
        const qreal zoomCoeffY = (m_d->canvasWidgetSize.height() - 2 * cfgMargin) / documentSize.height();

        KoZoomHandler::setZoom(this->zoom() * qMin(zoomCoeffX, zoomCoeffY));
        KoZoomHandler::setZoomMode(mode);
        recalculateTransformations();

        const QPointF offset = imageCenterInWidgetPixel() - widgetCenterPoint();

        QPointF newDocumentOffset = m_d->documentOffset + offset;

        // just explicitly set minimal axis offset to zero to
        // avoid imperfections of floating point numbers
        if (zoomCoeffX < zoomCoeffY) {
            newDocumentOffset.setX(-cfgMargin);
        } else {
            newDocumentOffset.setY(-cfgMargin);
        }

        setDocumentOffset(newDocumentOffset);
    }
}

qreal KisCoordinatesConverter::effectiveZoom() const
{
    qreal scaleX, scaleY;
    this->imageScale(&scaleX, &scaleY);

    if (scaleX != scaleY) {
        qWarning() << "WARNING: Zoom is not isotropic!"  << ppVar(scaleX) << ppVar(scaleY) << ppVar(qFuzzyCompare(scaleX, scaleY));
    }

    // zoom by average of x and y
    return 0.5 * (scaleX + scaleY);
}

qreal KisCoordinatesConverter::effectivePhysicalZoom() const
{
    qreal scaleX, scaleY;
    this->imagePhysicalScale(&scaleX, &scaleY);

    if (scaleX != scaleY) {
        qWarning() << "WARNING: Zoom is not isotropic!"  << ppVar(scaleX) << ppVar(scaleY) << ppVar(qFuzzyCompare(scaleX, scaleY));
    }

    // zoom by average of x and y
    return 0.5 * (scaleX + scaleY);
}

void KisCoordinatesConverter::enableNatureGestureFlag()
{
    m_d->isNativeGesture = true;
}

void KisCoordinatesConverter::beginRotation()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->isRotating);

    // Save the current transformation and angle to use as the base of the ongoing rotation.
    m_d->rotationBaseTransform = m_d->flakeToWidget;
    m_d->rotationBaseAngle = m_d->rotationAngle;
    m_d->isRotating = true;
}

void KisCoordinatesConverter::endRotation()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->isRotating);

    m_d->isNativeGesture = false;
    m_d->isRotating = false;
}

QPoint KisCoordinatesConverter::rotate(QPointF center, qreal angle)
{
    QTransform rot;
    rot.rotate(angle);

    if (!m_d->isNativeGesture && m_d->isRotating)
    {
        // Modal (begin/end) rotation. Transform from the stable base.
        m_d->flakeToWidget = m_d->rotationBaseTransform;
        m_d->rotationAngle = std::fmod(m_d->rotationBaseAngle + angle, 360.0);
    }
    else
    {
        // Immediate rotation, directly applied to the canvas transformation.
        m_d->rotationAngle = std::fmod(m_d->rotationAngle + angle, 360.0);
    }

    m_d->flakeToWidget *= QTransform::fromTranslate(-center.x(),-center.y());
    m_d->flakeToWidget *= rot;
    m_d->flakeToWidget *= QTransform::fromTranslate(center.x(), center.y());

    correctOffsetToTransformation();
    recalculateTransformations();

    return m_d->documentOffset.toPoint();
}

QPoint KisCoordinatesConverter::mirror(QPointF center, bool mirrorXAxis, bool mirrorYAxis)
{
    bool keepOrientation = false; // XXX: Keep here for now, maybe some day we can restore the parameter again.

    bool       doXMirroring = m_d->isXAxisMirrored ^ mirrorXAxis;
    bool       doYMirroring = m_d->isYAxisMirrored ^ mirrorYAxis;
    qreal      scaleX       = doXMirroring ? -1.0 : 1.0;
    qreal      scaleY       = doYMirroring ? -1.0 : 1.0;
    QTransform mirror       = QTransform::fromScale(scaleX, scaleY);

    QTransform rot;
    rot.rotate(m_d->rotationAngle);

    m_d->flakeToWidget *= QTransform::fromTranslate(-center.x(),-center.y());

    if (keepOrientation) {
        m_d->flakeToWidget *= rot.inverted();
    }

    m_d->flakeToWidget *= mirror;

    if (keepOrientation) {
        m_d->flakeToWidget *= rot;
    }

    m_d->flakeToWidget *= QTransform::fromTranslate(center.x(),center.y());


    if (!keepOrientation && (doXMirroring ^ doYMirroring)) {
        m_d->rotationAngle = -m_d->rotationAngle;
    }

    m_d->isXAxisMirrored = mirrorXAxis;
    m_d->isYAxisMirrored = mirrorYAxis;

    correctOffsetToTransformation();
    recalculateTransformations();

    return m_d->documentOffset.toPoint();
}

bool KisCoordinatesConverter::xAxisMirrored() const
{
    return m_d->isXAxisMirrored;
}

bool KisCoordinatesConverter::yAxisMirrored() const
{
    return m_d->isYAxisMirrored;
}

QPoint KisCoordinatesConverter::resetRotation(QPointF center)
{
    QTransform rot;
    rot.rotate(-m_d->rotationAngle);

    m_d->flakeToWidget *= QTransform::fromTranslate(-center.x(), -center.y());
    m_d->flakeToWidget *= rot;
    m_d->flakeToWidget *= QTransform::fromTranslate(center.x(), center.y());
    m_d->rotationAngle = 0.0;

    correctOffsetToTransformation();
    recalculateTransformations();

    return m_d->documentOffset.toPoint();
}

QTransform KisCoordinatesConverter::imageToWidgetTransform() const {
    return m_d->imageToDocument * m_d->documentToFlake * m_d->flakeToWidget;
}

QTransform KisCoordinatesConverter::imageToDocumentTransform() const {
    return m_d->imageToDocument;
}

QTransform KisCoordinatesConverter::documentToFlakeTransform() const {
    return m_d->documentToFlake;
}

QTransform KisCoordinatesConverter::flakeToWidgetTransform() const {
    return m_d->flakeToWidget;
}

QTransform KisCoordinatesConverter::documentToWidgetTransform() const {
    return m_d->documentToFlake * m_d->flakeToWidget;
}

QTransform KisCoordinatesConverter::viewportToWidgetTransform() const {
    return m_d->widgetToViewport.inverted();
}

QTransform KisCoordinatesConverter::imageToViewportTransform() const {
    return m_d->imageToDocument * m_d->documentToFlake * m_d->flakeToWidget * m_d->widgetToViewport;
}

void KisCoordinatesConverter::getQPainterCheckersInfo(QTransform *transform,
                                                      QPointF *brushOrigin,
                                                      QPolygonF *polygon,
                                                      const bool scrollCheckers) const
{
    /**
     * Qt has different rounding for QPainter::drawRect/drawImage.
     * The image is rounded mathematically, while rect in aligned
     * to the next integer. That causes transparent line appear on
     * the canvas.
     *
     * See: https://bugreports.qt.nokia.com/browse/QTBUG-22827
     */

    QRectF imageRect = imageRectInViewportPixels();
    imageRect.adjust(0,0,-0.5,-0.5);

    if (scrollCheckers) {
        *transform = viewportToWidgetTransform();
        *polygon = imageRect;
        *brushOrigin = imageToViewport(QPointF(0,0));
    }
    else {
        *transform = QTransform();
        *polygon = viewportToWidgetTransform().map(imageRect);
        *brushOrigin = QPoint(0,0);
    }
}

void KisCoordinatesConverter::getOpenGLCheckersInfo(const QRectF &viewportRect,
                                                    QTransform *textureTransform,
                                                    QTransform *modelTransform,
                                                    QRectF *textureRect,
                                                    QRectF *modelRect,
                                                    const bool scrollCheckers) const
{
    if(scrollCheckers) {
        *textureTransform = QTransform();
        *textureRect = QRectF(0, 0, viewportRect.width(),viewportRect.height());
    }
    else {
        *textureTransform = viewportToWidgetTransform();
        *textureRect = viewportRect;
    }

    *modelTransform = viewportToWidgetTransform();
    *modelRect = viewportRect;
}

QPointF KisCoordinatesConverter::imageCenterInWidgetPixel() const
{
    QPolygonF poly = imageToWidget(QPolygon(m_d->imageBounds));
    return (poly[0] + poly[1] + poly[2] + poly[3]) / 4.0;
}


// these functions return a bounding rect if the canvas is rotated

QRectF KisCoordinatesConverter::imageRectInWidgetPixels() const
{
    return imageToWidget(m_d->imageBounds);
}

QRectF KisCoordinatesConverter::imageRectInViewportPixels() const
{
    return imageToViewport(m_d->imageBounds);
}

QRect KisCoordinatesConverter::imageRectInImagePixels() const
{
    return m_d->imageBounds;
}

QRectF KisCoordinatesConverter::imageRectInDocumentPixels() const
{
    return imageToDocument(m_d->imageBounds);
}

QSizeF KisCoordinatesConverter::imageSizeInFlakePixels() const
{
    qreal scaleX, scaleY;
    imageScale(&scaleX, &scaleY);
    QSize imageSize = m_d->imageBounds.size();

    return QSizeF(imageSize.width() * scaleX, imageSize.height() * scaleY);
}

QRectF KisCoordinatesConverter::widgetRectInFlakePixels() const
{
    return widgetToFlake(QRectF(QPoint(0,0), m_d->canvasWidgetSize));
}

QRectF KisCoordinatesConverter::widgetRectInImagePixels() const
{
    return widgetToImage(QRectF(QPoint(0,0), m_d->canvasWidgetSize));
}

QPointF KisCoordinatesConverter::flakeCenterPoint() const
{
    QRectF widgetRect = widgetRectInFlakePixels();
    return QPointF(widgetRect.left() + widgetRect.width() / 2,
                   widgetRect.top() + widgetRect.height() / 2);
}

QPointF KisCoordinatesConverter::widgetCenterPoint() const
{
    return QPointF(m_d->canvasWidgetSize.width() / 2.0, m_d->canvasWidgetSize.height() / 2.0);
}

void KisCoordinatesConverter::imageScale(qreal *scaleX, qreal *scaleY) const
{
    // get the x and y zoom level of the canvas
    qreal zoomX, zoomY;
    KoZoomHandler::zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal resX = m_d->imageXRes;
    qreal resY = m_d->imageYRes;

    // Compute the scale factors
    *scaleX = zoomX / resX;
    *scaleY = zoomY / resY;
}

void KisCoordinatesConverter::imagePhysicalScale(qreal *scaleX, qreal *scaleY) const
{
    imageScale(scaleX, scaleY);
    *scaleX *= m_d->devicePixelRatio;
    *scaleY *= m_d->devicePixelRatio;
}

/**
 * @brief Adjust a given pair of coordinates to the nearest device pixel
 *        according to the value of `devicePixelRatio`.
 * @param point a point in logical pixel space
 * @return The point in logical pixel space but adjusted to the nearest device
 *         pixel
 */

QPointF KisCoordinatesConverter::snapToDevicePixel(const QPointF &point) const
{
    QPoint devicePixel = (point * m_d->devicePixelRatio).toPoint();
    // These adjusted coords will be in logical pixel but is aligned in device
    // pixel space for pixel-perfect rendering.
    return QPointF(devicePixel) / m_d->devicePixelRatio;
}

QTransform KisCoordinatesConverter::viewToWidget() const
{
    return flakeToWidgetTransform();
}

QTransform KisCoordinatesConverter::widgetToView() const
{
    return flakeToWidgetTransform().inverted();
}
