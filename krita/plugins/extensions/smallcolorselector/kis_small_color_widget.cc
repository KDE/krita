/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_small_color_widget.h"
#include <QMouseEvent>
#include <QPixmap>
#include <QPainter>

#include <KoColorConversions.h>

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
};

KisSmallColorWidget::KisSmallColorWidget(QWidget* parent) : QWidget(parent), d(new Private)
{
    setMinimumHeight(50);
    d->hue = 0;
    d->value = 0;
    d->saturation = 0;
    d->updateAllowed = true;
    d->handle = NoHandle;
    updateParameters();
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
    update();
}

void KisSmallColorWidget::setHSV(int h, int s, int v)
{
    h = qBound(0, h, 360);
    s = qBound(0, s, 255);
    v = qBound(0, v, 255);
    d->hue = h;
    d->value = v;
    d->saturation = s;
    tellColorChanged();
    generateSquare();
    update();
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
        update();
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
                d->value * d->rectangleHeight / 255.0);
    p.drawEllipse(QRectF(-d->squareHandleSize * 0.5, -d->squareHandleSize * 0.5, d->squareHandleSize, d->squareHandleSize));
    p.end();
}

void KisSmallColorWidget::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);
    setMaximumHeight(50);
    updateParameters();
    generateRubber();
    generateSquare();
}

void KisSmallColorWidget::updateParameters()
{
    d->margin = 5;
    d->rectangleWidthProportion = 0.3;
    d->rectangleWidth = qMax((int)(width() * d->rectangleWidthProportion) , height());
    d->rectangleHeight = height();
    d->rubberWidth = width() - d->rectangleWidth - d->margin;
    d->rubberHeight = height();
    d->squareHandleSize = 10.0;
}

void KisSmallColorWidget::generateRubber()
{
    QImage img(d->rubberWidth, d->rubberHeight, QImage::Format_RGB32);
    for (int y = 0; y < d->rubberHeight; y++) {
        for (int x = 0; x < d->rubberWidth; x++) {
            int h = (x * 360) / d->rubberWidth ;
            int r, g, b;
            hsv_to_rgb(h, 255, 255, &r, &g, &b);
            img.setPixel(x, y, qRgb(r, g, b));
        }
    }
    d->rubberPixmap = QPixmap::fromImage(img);
}

void KisSmallColorWidget::generateSquare()
{
    QImage img(d->rectangleWidth, d->rectangleHeight, QImage::Format_RGB32);
    for (int y = 0; y < d->rectangleHeight; y++) {
        int v = (y * 255) / d->rectangleHeight;
        for (int x = 0; x < d->rectangleWidth; x++) {
            int s = (x * 255) / d->rectangleWidth;
            int r, g, b;
            hsv_to_rgb(hue(), s, v, &r, &g, &b);
            img.setPixel(x, y, qRgb(r, g, b));
        }
    }
    d->squarePixmap = QPixmap::fromImage(img);
}

void KisSmallColorWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        selectColorAt(event->x(), event->y());
        d->handle = NoHandle;
    }
    QWidget::mouseReleaseEvent(event);
}

void KisSmallColorWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        d->handle = NoHandle;
        selectColorAt(event->x(), event->y());
    }
    QWidget::mousePressEvent(event);
}

void KisSmallColorWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton) {
        selectColorAt(event->x(), event->y());
    }
    QWidget::mouseMoveEvent(event);
}

void KisSmallColorWidget::selectColorAt(int _x, int _y)
{
    if ((_x < d->rubberWidth && d->handle == NoHandle) || d->handle == HueHandle) {
        d->handle = HueHandle;
        setHue((_x * 360.0) / d->rubberWidth);
        update();
    } else if ((_x > width() - d->rectangleWidth && d->handle == NoHandle) || d->handle == ValueSaturationHandle) {
        d->handle = ValueSaturationHandle;
        setHSV(d->hue, (_x - width() + d->rectangleWidth) * 255 / d->rectangleWidth, (_y * 255) / d->rectangleHeight);
        update();
    }
}

#include "kis_small_color_widget.moc"
