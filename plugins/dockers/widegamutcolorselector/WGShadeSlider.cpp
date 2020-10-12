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
    qreal handleValue {0};
    qreal leftStart;
    qreal leftEnd;
    qreal rightStart;
    qreal rightEnd;
    KisVisualColorModel *selectorModel {0};
    int cursorWidth {11};
    int numPatches {9};
    bool sliderMode {true};
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
    m_d->imageNeedsUpdate = true;
    resetHandle();
}

void WGShadeSlider::setDisplayMode(bool slider, int numPatches)
{
    if (slider != m_d->sliderMode) {
        m_d->sliderMode = slider;
        if (!slider && numPatches > 2) {
            m_d->numPatches = numPatches;
        }
        m_d->imageNeedsUpdate = true;
        resetHandle();
    }
}

QVector4D WGShadeSlider::channelValues() const
{
    return calculateChannelValues(m_d->handleValue);
}

QSize WGShadeSlider::minimumSizeHint() const
{
    return QSize(50, 8);
}

void WGShadeSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sigInteraction(true);
        if (adjustHandleValue(event->localPos())) {
            emit sigChannelValuesChanged(channelValues());
            update();
        }
    } else {
        event->ignore();
    }
}

void WGShadeSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (adjustHandleValue(event->localPos())) {
            emit sigChannelValuesChanged(channelValues());
            update();
        }
    } else {
        event->ignore();
    }
}

void WGShadeSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sigInteraction(false);
    } else {
        event->ignore();
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
    if (m_d->sliderMode) {
        QPointF sliderPos = convertSliderValueToWidgetCoordinate(m_d->handleValue);
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
    } else {
        if (m_d->handleValue >= 0) {
            painter.translate(0.5, 0.5);
            QRectF handle = patchRect(m_d->handleValue).adjusted(0, 0, -1, -1);
            painter.setPen(QColor(175,175,175));
            painter.drawRect(handle.adjusted(-1, 0, 1, 0));
            painter.setPen(QColor(75,75,75));
            painter.drawRect(handle);
        }
    }
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
    m_d->imageNeedsUpdate = true;
    resetHandle();
}

void WGShadeSlider::resetHandle()
{
    m_d->handleValue = m_d->sliderMode ? 0 : -1;
    update();
}

bool WGShadeSlider::adjustHandleValue(const QPointF &widgetPos)
{
    if (m_d->sliderMode) {
        qreal sliderPos = convertWidgetCoordinateToSliderValue(widgetPos);
        if (!qFuzzyIsNull(m_d->handleValue - sliderPos)) {
            m_d->handleValue = sliderPos;
            return true;
        }
    } else {
        int patchNum = getPatch(widgetPos);
        if (patchNum >= 0 && patchNum != (int)m_d->handleValue) {
            m_d->handleValue = patchNum;
            return true;
        }
    }
    return false;
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
    float delta = 0.0f;
    if (m_d->sliderMode) {
        delta = (float)sliderPos;
    } else if (sliderPos >= 0 || m_d->numPatches > 1) {
        delta = 2.0f * float(sliderPos)/(m_d->numPatches - 1.0f) - 1.0f;
    }

    QVector4D coordinates = m_d->baseValues + delta * m_d->gradient;
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

int WGShadeSlider::getPatch(const QPointF pos) const
{
    int patch = m_d->numPatches * pos.x() / width();
    if (patch >= 0 && patch < m_d->numPatches) {
        return patch;
    }
    return -1;
}

QRectF WGShadeSlider::patchRect(int index) const
{
    qreal patchWidth = width() / qreal(m_d->numPatches);
    qreal margin = 1.5;
    QPointF topLeft(index * patchWidth + margin, 0);
    QPointF bottomRight((index+1) * patchWidth - margin, height());
    return QRectF(topLeft, bottomRight);
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
    if (m_d->sliderMode) {
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
    } else {
        QImage image(deviceWidth, deviceHeight, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        image.setDevicePixelRatio(devicePixelRatioF());
        QPainter painter(&image);
        painter.setPen(Qt::NoPen);

        for (int i = 0; i < m_d->numPatches; i++) {
            QVector4D values = calculateChannelValues(i);
            KoColor col = m_d->selectorModel->convertChannelValuesToKoColor(values);
            QColor qCol = m_d->selectorModel->displayRenderer()->toQColor(col);
            painter.setBrush(qCol);
            painter.drawRect(patchRect(i));
        }
        return image;
    }
}
