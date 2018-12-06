/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_small_color_widget.h"
#include <QMouseEvent>
#include <QPixmap>
#include <QPainter>
#include <QTimer>

#include <KoColorConversions.h>

#include "kis_debug.h"

enum CurrentHandle {
    NoHandle,
    HueHandle,
    ValueSaturationHandle
};

struct KisSmallColorWidget::Private {
    QPixmap rubberPixmap;
    QPixmap squarePixmap;
    double rectangleWidthProportion;
    int rectangleHeight;
    int rectangleWidth;
    int rubberWidth;
    int rubberHeight;
    int margin;
    int hue;
    int value;
    int saturation;
    bool updateAllowed;
    double squareHandleSize;
    CurrentHandle handle;
    int lastX, lastY;
    QTimer updateTimer;
};

KisSmallColorWidget::KisSmallColorWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      d(new Private)
{
    setTextureFormat(GL_RGBA16F);

    d->hue = 0;
    d->value = 0;
    d->saturation = 0;
    d->updateAllowed = true;
    d->handle = NoHandle;
    updateParameters(QSize(1,1));
    d->lastX = -1;
    d->lastY = -1;
    d->updateTimer.setInterval(1);
    d->updateTimer.setSingleShot(true);
    connect(&(d->updateTimer), SIGNAL(timeout()), this, SLOT(update()));

    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

KisSmallColorWidget::~KisSmallColorWidget()
{
    delete d;
}

int KisSmallColorWidget::hue() const
{
    return d->hue;
}

int KisSmallColorWidget::value() const
{
    return d->value;
}

int KisSmallColorWidget::saturation() const
{
    return d->saturation;
}

QColor KisSmallColorWidget::color() const
{
    int r, g, b;
    hsv_to_rgb(d->hue, d->saturation, d->value, &r, &g, &b);
    return QColor(r, g, b);
}

void KisSmallColorWidget::setHue(int h)
{
    h = qBound(0, h, 360);
    d->hue = h;
    tellColorChanged();
    generateSquare();
    d->updateTimer.start();
}

void KisSmallColorWidget::setHSV(int h, int s, int v)
{
    h = qBound(0, h, 360);
    s = qBound(0, s, 255);
    v = qBound(0, v, 255);
    bool newH = (d->hue != h);
    d->hue = h;
    d->value = v;
    d->saturation = s;
    tellColorChanged();
    if(newH) {
        generateSquare();
    }
    d->updateTimer.start();
}

void KisSmallColorWidget::setQColor(const QColor& c)
{
    if (d->updateAllowed) {
        int hue;
        rgb_to_hsv(c.red(), c.green(), c.blue(), &hue, &d->saturation, &d->value);
        if (hue >= 0 && hue <= 360) {
            d->hue = hue;
        }
        generateSquare();
        d->updateTimer.start();
    }
}

void KisSmallColorWidget::tellColorChanged()
{
    d->updateAllowed = false;
    emit(colorChanged(color()));
    d->updateAllowed = true;
}

void KisSmallColorWidget::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPixmap(0, 0, d->rubberPixmap);
    p.drawPixmap(width() - d->rectangleWidth, 0 , d->squarePixmap);
    // Draw Hue handle
    p.save();
    p.setPen(QPen(Qt::white, 1.0));
    p.translate((d->hue * d->rubberWidth) / 360.0 , 0.0);
    p.drawRect(QRectF(-1.5, 0 , 3.0, height()));
    p.restore();
    // Draw Saturation / Value handle
    p.setPen(QPen(Qt::white, 1.0));
    p.setBrush(color());
    p.translate(d->saturation * d->rectangleWidth / 255.0 + width() - d->rectangleWidth,
                d->rectangleHeight-(d->value * d->rectangleHeight / 255.0));
    p.drawEllipse(QRectF(-d->squareHandleSize * 0.5, -d->squareHandleSize * 0.5, d->squareHandleSize, d->squareHandleSize));
    p.end();
}

QSize KisSmallColorWidget::sizeHint() const
{
    const int preferredWidth = 200;
    return QSize(preferredWidth, heightForWidth(preferredWidth));
}

bool KisSmallColorWidget::hasHeightForWidth() const
{
    return true;
}

int KisSmallColorWidget::heightForWidth(int width) const
{
    return d->rectangleWidthProportion * width + 2 * d->margin;
}

void KisSmallColorWidget::resizeEvent(QResizeEvent * event)
{
    QOpenGLWidget::resizeEvent(event);

    updateParameters(event->size());
    generateRubber();
    generateSquare();

    update();
}

void KisSmallColorWidget::updateParameters(const QSize &size)
{
    d->margin = 5;
    d->rectangleWidthProportion = 0.3;
    d->rectangleWidth = qMax((int)(size.width() * d->rectangleWidthProportion) , size.height());
    d->rectangleHeight = size.height();
    d->rubberWidth = size.width() - d->rectangleWidth - d->margin;
    d->rubberHeight = size.height();
    d->squareHandleSize = 10.0;
}

void KisSmallColorWidget::generateRubber()
{
    QImage image(d->rubberWidth, d->rubberHeight, QImage::Format_RGB32);
    for (int y = 0; y < d->rubberHeight; y++) {
        for (int x = 0; x < d->rubberWidth; x++) {
            int h = (x * 360) / d->rubberWidth ;
            int r, g, b;
            hsv_to_rgb(h, 255, 255, &r, &g, &b);
            image.setPixel(x, y, qRgb(r, g, b));
        }
    }
    d->rubberPixmap = QPixmap::fromImage(image);
}

void KisSmallColorWidget::generateSquare()
{
    QImage image(d->rectangleWidth, d->rectangleHeight, QImage::Format_RGB32);
    for (int y = 0; y < d->rectangleHeight; ++y) {
        int v = 255-((y * 255) / d->rectangleHeight);
        uint* data = reinterpret_cast<uint*>(image.scanLine(y));
        for (int x = 0; x < d->rectangleWidth; ++x, ++data) {
            int s = (x * 255) / d->rectangleWidth;
            int r, g, b;
            hsv_to_rgb(hue(), s, v, &r, &g, &b);
            *data = qRgb(r, g, b);
        }
    }
    d->squarePixmap = QPixmap::fromImage(image);
}

void KisSmallColorWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        selectColorAt(event->x(), event->y());
        d->handle = NoHandle;
    } else {
        QOpenGLWidget::mouseReleaseEvent(event);
    }
}

void KisSmallColorWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        d->handle = NoHandle;
        selectColorAt(event->x(), event->y());
    } else {
        QOpenGLWidget::mousePressEvent(event);
    }
}

void KisSmallColorWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton) {
        selectColorAt(event->x(), event->y());
    } else {
        QOpenGLWidget::mouseMoveEvent(event);
    }
}

void KisSmallColorWidget::selectColorAt(int _x, int _y)
{
    if (d->lastX == _x && d->lastY == _y)
    {
        return;
    }
    d->lastX = _x;
    d->lastY = _y;
    if ((_x < d->rubberWidth && d->handle == NoHandle) || d->handle == HueHandle) {
        d->handle = HueHandle;
        setHue((_x * 360.0) / d->rubberWidth);
        d->updateTimer.start();
    } else if ((_x > width() - d->rectangleWidth && d->handle == NoHandle) || d->handle == ValueSaturationHandle) {
        d->handle = ValueSaturationHandle;
        setHSV(d->hue, (_x - width() + d->rectangleWidth) * 255 / d->rectangleWidth, 255-((_y * 255) / d->rectangleHeight));
        d->updateTimer.start();
    }
}

