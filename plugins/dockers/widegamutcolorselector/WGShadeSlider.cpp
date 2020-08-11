/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGShadeSlider.h"

#include "KisVisualColorModel.h"
#include "KoColorDisplayRendererInterface.h"

#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QVector4D>
#include <QtMath>

struct WGShadeSlider::Private
{
    Private() {}
    QImage background;
    QVector4D gradient;
    QVector4D baseValues;
    qreal sliderValue {0};
    qreal leftStart;
    qreal leftEnd;
    qreal rightStart;
    qreal rightEnd;
    KisVisualColorModel *selectorModel {0};
    int cursorWidth {11};
    bool imageNeedsUpdate {true};
};

WGShadeSlider::WGShadeSlider(QWidget *parent, KisVisualColorModel *model)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->selectorModel = model;
    //test
    m_d->baseValues = QVector4D(0.5, 0.5, 0.5, 0.0);
    m_d->gradient = QVector4D(0.0, -0.3, 0.3, 0.0);
}

WGShadeSlider::~WGShadeSlider()
{}

void WGShadeSlider::setGradient(const QVector4D &gradient)
{
    m_d->gradient = gradient;
    if (m_d->sliderValue != 0) {
        emit sigChannelValuesChanged(channelValues());
    }
    m_d->imageNeedsUpdate = true;
    update();
}

QVector4D WGShadeSlider::channelValues() const
{
    return calculateChannelValues(m_d->sliderValue);
}

QSize WGShadeSlider::minimumSizeHint() const
{
    return QSize(50, 8);
}

void WGShadeSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sigInteraction(true);
        qreal sliderPos = convertWidgetCoordinateToSliderValue(event->localPos());
        if (!qFuzzyIsNull(m_d->sliderValue - sliderPos)) {
            m_d->sliderValue = sliderPos;
            emit sigChannelValuesChanged(channelValues());
            update();
        }
    }
}

void WGShadeSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        qreal sliderPos = convertWidgetCoordinateToSliderValue(event->localPos());
        if (!qFuzzyIsNull(m_d->sliderValue - sliderPos)) {
            m_d->sliderValue = sliderPos;
            emit sigChannelValuesChanged(channelValues());
            update();
        }
    }
}

void WGShadeSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sigInteraction(false);
    }
}

void WGShadeSlider::paintEvent(QPaintEvent*)
{
    if (m_d->imageNeedsUpdate) {
        m_d->background = renderBackground();
        m_d->imageNeedsUpdate = false;
    }
    QPainter painter(this);
    painter.drawImage(0, 0, m_d->background);
    // TODO: proper handle
    QPointF sliderPos = convertSliderValueToWidgetCoordinate(m_d->sliderValue);
    int sliderX = qRound(sliderPos.x());
    //painter.drawLine(sliderPos, QPointF(sliderPos.x(), height()));
    painter.translate(0.5, 0.5);
    painter.setPen(QColor(175,175,175));
    painter.drawRect(m_d->leftStart, 0, m_d->cursorWidth - 1, height() - 1);
    painter.setPen(QColor(75,75,75));
    painter.drawRect(m_d->leftStart + 1, 0, m_d->cursorWidth - 3, height() - 1);
    painter.setPen(QColor(175,175,175));
    painter.drawRect(sliderX - m_d->cursorWidth/2, 0, m_d->cursorWidth - 1, height() - 1);
    painter.setPen(QColor(75,75,75));
    painter.drawRect(sliderX - m_d->cursorWidth/2 + 1, 0, m_d->cursorWidth - 3, height() - 1);
}

void WGShadeSlider::resizeEvent(QResizeEvent *)
{
    int center = (width() - 1) / 2;
    int halfCursor = m_d->cursorWidth / 2;
    m_d->leftEnd = halfCursor;
    m_d->leftStart = center - halfCursor;
    //m_d->rightStart = m_d->leftStart + m_d->cursorWidth;
    m_d->rightStart = center + halfCursor;
    //m_d->rightEnd = 2 * center + m_d->cursorWidth - 3 * halfCursor;
    m_d->rightEnd = 2 * center  - halfCursor;
    m_d->imageNeedsUpdate = true;
}

void WGShadeSlider::slotSetChannelValues(const QVector4D &values)
{
    m_d->baseValues = values;
    m_d->sliderValue = 0.0;
    m_d->imageNeedsUpdate = true;
    update();
}

void WGShadeSlider::setSliderValue(qreal value)
{
    m_d->sliderValue = qBound(-1.0, value, 1.0);
    update();
}

QPointF WGShadeSlider::convertSliderValueToWidgetCoordinate(qreal value)
{
    QPointF pos(0.0, 0.0);
    if (value < 0) {
        pos.setX(m_d->leftStart - value * (m_d->leftEnd - m_d->leftStart));
    }
    else if (value > 0) {
        pos.setX(m_d->rightStart + value * (m_d->rightEnd - m_d->rightStart));
    }
    else {
        pos.setX((width() - 1) / 2);
    }
    return pos;
}

qreal WGShadeSlider::convertWidgetCoordinateToSliderValue(QPointF coordinate)
{
    qreal x = coordinate.x();
    if (x < m_d->leftEnd) {
        return -1.0;
    }
    else if (x < m_d->leftStart) {
        return  (m_d->leftStart - x) / (m_d->leftEnd - m_d->leftStart);
    }
    else if (x < m_d->rightStart) {
        return 0.0;
    }
    else if (x < m_d->rightEnd) {
        return (x - m_d->rightStart) / (m_d->rightEnd - m_d->rightStart);
    }
    return 1.0;
}

QVector4D WGShadeSlider::calculateChannelValues(qreal sliderPos) const
{
    QVector4D coordinates = m_d->baseValues + sliderPos * m_d->gradient;
    // Hue wraps around
    coordinates[0] = (float)fmod(coordinates[0], 1.0);
    if (coordinates[0] < 0) {
        coordinates[0] += 1.f;
    }
    for (int i = 1; i < 3; i++) {
        coordinates[i] = qBound(0.f, coordinates[i], 1.f);
    }
    return coordinates;
}

QImage WGShadeSlider::renderBackground()
{
    if (! m_d->selectorModel || !m_d->selectorModel->colorSpace()) {
        return QImage();
    }

    // Hi-DPI aware rendering requires that we determine the device pixel dimension;
    // actual widget size in device pixels is not accessible unfortunately, it might be 1px smaller...
    const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const int deviceWidth = qCeil(width() * devicePixelRatioF());
    const int deviceHeight = qCeil(height() * devicePixelRatioF());
    const KoColorSpace *currentCS = m_d->selectorModel->colorSpace();
    const quint32 pixelSize = currentCS->pixelSize();
    quint32 imageSize = deviceWidth * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize] {});
    quint8 *dataPtr = raw.data();

    for (int x = 0; x < deviceWidth; x++, dataPtr += pixelSize) {
        qreal sliderVal = convertWidgetCoordinateToSliderValue(QPointF(x, 0) * deviceDivider);
        QVector4D coordinates = calculateChannelValues(sliderVal);
        KoColor c = m_d->selectorModel->convertChannelValuesToKoColor(coordinates);
        memcpy(dataPtr, c.data(), pixelSize);
    }

    QImage image = m_d->selectorModel->displayRenderer()->convertToQImage(currentCS, raw.data(), deviceWidth, 1);
    image = image.scaled(QSize(deviceWidth, deviceHeight));
    image.setDevicePixelRatio(devicePixelRatioF());

    return image;
}
