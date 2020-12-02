/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualEllipticalSelectorShape.h"

#include <QColor>
#include <QPainter>
#include <QRect>
#include <QLineF>
#include <QtMath>

#include "kis_debug.h"
#include "kis_global.h"

#include "resources/KoGamutMask.h"

#define KVESS_MARGIN 2

KisVisualEllipticalSelectorShape::KisVisualEllipticalSelectorShape(KisVisualColorSelector *parent,
                                                                 Dimensions dimension,
                                                                 int channel1, int channel2,
                                                                 int barWidth,
                                                                 singelDTypes d)
    : KisVisualColorSelectorShape(parent, dimension, channel1, channel2)
{
    //qDebug() << "creating KisVisualEllipticalSelectorShape" << this;
    m_type = d;
    m_barWidth = barWidth;
    m_gamutMaskNeedsUpdate = (dimension == KisVisualColorSelectorShape::twodimensional);
}

KisVisualEllipticalSelectorShape::~KisVisualEllipticalSelectorShape()
{
    //qDebug() << "deleting KisVisualEllipticalSelectorShape" << this;
}

void KisVisualEllipticalSelectorShape::setBorderWidth(int width)
{
    m_barWidth = width;
    forceImageUpdate();
    update();
}

QRect KisVisualEllipticalSelectorShape::getSpaceForSquare(QRect geom)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(geom.contains(geometry()), geom);
    int sizeValue = qMin(width(), height());
    QRectF b(0, 0, sizeValue, sizeValue);
    QLineF radius(b.center(), QPointF(b.left() + m_barWidth - 1, b.center().y()) );
    radius.setAngle(135);
    QPointF tl(qFloor(radius.p2().x()), qFloor(radius.p2().y()));
    QPointF br = b.bottomRight() - tl;
    // QRect interprets bottomRight differently (unsuitable) for "historical reasons",
    // so construct a QRectF and convert to QRect
    QRect r = QRectF(tl, br).toRect();
    r.translate(pos());
    return r;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForCircle(QRect geom)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(geom.contains(geometry()), geom);
    int sizeValue = qMin(width(), height());
    QRect b(0, 0, sizeValue, sizeValue);
    QPointF tl = QPointF (b.topLeft().x()+m_barWidth, b.topLeft().y()+m_barWidth);
    QPointF br = QPointF (b.bottomRight().x()-m_barWidth, b.bottomRight().y()-m_barWidth);
    QRect r(tl.toPoint(), br.toPoint());
    r.translate(pos());
    return r;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForTriangle(QRect geom)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(geom.contains(geometry()), geom);
    int sizeValue = qMin(width(), height());
    QPointF center(0.5 * width(), 0.5 * height());
    qreal radius = 0.5 * sizeValue - (m_barWidth + 4);
    QLineF rLine(center, QPointF(center.x() + radius, center.y()));
    rLine.setAngle(330);
    QPoint br(rLine.p2().toPoint());
    QPoint tl(width() - br.x(), m_barWidth + 4);
    // can't use QRect(tl, br) constructor because it interprets br unsuitably for "historical reasons"
    QRect bound(tl, QSize(br.x() - tl.x(), br.y() - tl.y()));
    bound.adjust(-5, -5, 5, 5);
    bound.translate(pos());
    return bound;
}

bool KisVisualEllipticalSelectorShape::supportsGamutMask() const
{
    return (getDimensions() == KisVisualColorSelectorShape::twodimensional);
}

void KisVisualEllipticalSelectorShape::updateGamutMask()
{
    if (supportsGamutMask()) {
        m_gamutMaskNeedsUpdate = true;
        KoGamutMask *mask = colorSelector()->activeGamutMask();
        if (mask) {
            m_gamutMaskTransform = mask->viewToMaskTransform(width() - 2*KVESS_MARGIN);
            m_gamutMaskTransform.translate(-KVESS_MARGIN, -KVESS_MARGIN);
        }
        update();
    }
}

QPointF KisVisualEllipticalSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const
{
    qreal offset = 7.0;
    qreal a = (qreal)width()*0.5;
    QPointF center(a, a);
    QLineF line(center, QPoint((m_barWidth*0.5),a));
    qreal angle = coordinate.x()*360.0;
    angle = 360.0 - fmod(angle+180.0, 360.0);
    if (m_type==KisVisualEllipticalSelectorShape::borderMirrored) {
        angle = (coordinate.x()/2)*360.0;
        angle = fmod((angle+270.0), 360.0);
    }
    line.setAngle(angle);
    if (getDimensions()!=KisVisualColorSelectorShape::onedimensional) {
        line.setLength(qMin(coordinate.y()*(a-offset), a-offset));
    }
    return line.p2();
}

QPointF KisVisualEllipticalSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const
{
    //default implementation:
    qreal x = 0.5;
    qreal y = 1.0;
    qreal offset = 7.0;
    QPointF center = QRectF(QPointF(0.0, 0.0), this->size()).center();
    qreal a = (qreal(this->width()) / qreal(2));
    qreal xRel = center.x()-coordinate.x();
    qreal yRel = center.y()-coordinate.y();
    qreal radius = sqrt(xRel*xRel+yRel*yRel);

    if (m_type!=KisVisualEllipticalSelectorShape::borderMirrored){
        qreal angle = atan2(yRel, xRel);
        angle = kisRadiansToDegrees(angle);
        angle = fmod(angle+360, 360.0);
        x = angle/360.0;
        if (getDimensions()==KisVisualColorSelectorShape::twodimensional) {
            y = qBound(0.0,radius/(a-offset), 1.0);
        }

    } else {
        qreal angle = atan2(xRel, yRel);
        angle = kisRadiansToDegrees(angle);
        angle = fmod(angle+180, 360.0);
        if (angle>180.0) {
            angle = 360.0-angle;
        }
        x = (angle/360.0)*2;
        if (getDimensions()==KisVisualColorSelectorShape::twodimensional) {
            y = qBound(0.0,(radius+offset)/a, 1.0);
        }
    }

    return QPointF(x, y);
}

QPointF KisVisualEllipticalSelectorShape::mousePositionToShapeCoordinate(const QPointF &pos, const QPointF &dragStart) const
{
    QPointF pos2(pos);
    if (m_type == KisVisualEllipticalSelectorShape::borderMirrored) {
        qreal h_center = width()/2.0;
        bool start_left = dragStart.x() < h_center;
        bool cursor_left = pos.x() < h_center;
        if (start_left != cursor_left) {
            pos2.setX(h_center);
        }
    }
    else if (getDimensions() == KisVisualColorSelectorShape::twodimensional) {
        KoGamutMask *mask = colorSelector()->activeGamutMask();
        if (mask) {
            QPointF maskPoint = m_gamutMaskTransform.map(pos);
            if (!mask->coordIsClear(maskPoint, true)) {
                // Ideally we try  to find the closest point on the mask border, possibly
                // depending on dragStart. Currently just returns old position.
                return getCursorPosition();
            }
        }
    }
    return convertWidgetCoordinateToShapeCoordinate(pos2);
}

QRegion KisVisualEllipticalSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(0,0,width(),height(), QRegion::Ellipse);
    if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
        mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2), QRegion::Ellipse));
    }
    return mask;
}

QImage KisVisualEllipticalSelectorShape::renderAlphaMaskImpl(qreal outerBorder, qreal innerBorder) const
{
    // Hi-DPI aware rendering requires that we determine the device pixel dimension;
    // actual widget size in device pixels is not accessible unfortunately, it might be 1px smaller...
    const int deviceWidth = qCeil(width() * devicePixelRatioF());
    const int deviceHeight = qCeil(height() * devicePixelRatioF());

    QImage alphaMask(deviceWidth, deviceHeight, QImage::Format_Alpha8);
    alphaMask.fill(0);
    alphaMask.setDevicePixelRatio(devicePixelRatioF());
    QPainter painter(&alphaMask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    QRectF circle(outerBorder, outerBorder, width() - 2*outerBorder, height() - 2*outerBorder);
    painter.drawEllipse(circle);

    //painter.setBrush(Qt::black);
    if (innerBorder > outerBorder) {
        circle = QRectF(innerBorder, innerBorder, width() - 2*innerBorder, height() - 2*innerBorder);
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.drawEllipse(circle);
    }
    return alphaMask;
}

QImage KisVisualEllipticalSelectorShape::renderAlphaMask() const
{
    KisVisualColorSelector::RenderMode mode = colorSelector()->renderMode();
    if (isHueControl() && mode == KisVisualColorSelector::StaticBackground) {
        return QImage();
    }
    qreal outerBorder = KVESS_MARGIN;
    qreal innerBorder = -1;
    if (mode == KisVisualColorSelector::CompositeBackground && isHueControl()) {
        outerBorder += 0.25 * (m_barWidth - 4);
    }
    if (getDimensions() == KisVisualColorSelectorShape::onedimensional) {
        innerBorder = m_barWidth - 2;
    }
    return renderAlphaMaskImpl(outerBorder, innerBorder);
}

QImage KisVisualEllipticalSelectorShape::renderStaticAlphaMask() const
{
    KisVisualColorSelector::RenderMode mode = colorSelector()->renderMode();
    if (!isHueControl() || mode == KisVisualColorSelector::DynamicBackground) {
        return QImage();
    }
    qreal innerBorder = m_barWidth - 2;
    if (mode == KisVisualColorSelector::CompositeBackground) {
        innerBorder = KVESS_MARGIN + 1 + 0.25 * (m_barWidth - 4);
    }
    return renderAlphaMaskImpl(KVESS_MARGIN, innerBorder);
}

void KisVisualEllipticalSelectorShape::renderGamutMask()
{
    KoGamutMask *mask = colorSelector()->activeGamutMask();

    if (!mask) {
        m_gamutMaskImage = QImage();
        return;
    }
    const int deviceWidth = qCeil(width() * devicePixelRatioF());
    const int deviceHeight = qCeil(height() * devicePixelRatioF());

    if (m_gamutMaskImage.size() != QSize(deviceWidth, deviceHeight)) {
        m_gamutMaskImage = QImage(deviceWidth, deviceHeight, QImage::Format_ARGB32_Premultiplied);
        m_gamutMaskImage.setDevicePixelRatio(devicePixelRatioF());
    }
    m_gamutMaskImage.fill(0);

    QPainter painter(&m_gamutMaskImage);
    QPen pen(Qt::white);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(KVESS_MARGIN, KVESS_MARGIN);
    painter.setBrush(QColor(0, 0, 0, 128));
    painter.setPen(pen);

    painter.drawEllipse(QRectF(0, 0, width() - 2*KVESS_MARGIN, height() - 2*KVESS_MARGIN));

    painter.setTransform(mask->maskToViewTransform(width() - 2*KVESS_MARGIN), true);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    mask->paint(painter, true);

    // TODO: implement a way to render gamut mask outline with custom pen
    // determine how many units 1 pixel is now:
    //QLineF measure = painter.transform().map(QLineF(0.0, 0.0, 1.0, 0.0));
    //pen.setWidthF(1.0 / measure.length());
    //painter.setPen(pen);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    mask->paintStroke(painter, true);

    m_gamutMaskNeedsUpdate = false;
}

void KisVisualEllipticalSelectorShape::drawCursor(QPainter &painter)
{
    //qDebug() << this << "KisVisualEllipticalSelectorShape::drawCursor: image needs update" << imagesNeedUpdate();
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QColor col = getColorFromConverter(getCurrentColor());
    QBrush fill(Qt::SolidPattern);

    int cursorwidth = 5;

    if (m_type==KisVisualEllipticalSelectorShape::borderMirrored) {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        QPointF mirror(width() - cursorPoint.x(), cursorPoint.y());
        painter.drawEllipse(mirror, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1, cursorwidth-1);
        painter.drawEllipse(mirror, cursorwidth-1, cursorwidth-1);

    } else {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1.0, cursorwidth-1.0);
    }
}

void KisVisualEllipticalSelectorShape::drawGamutMask(QPainter &painter)
{
    if (m_gamutMaskNeedsUpdate) {
        renderGamutMask();
    }
    painter.drawImage(0, 0, m_gamutMaskImage);
}
