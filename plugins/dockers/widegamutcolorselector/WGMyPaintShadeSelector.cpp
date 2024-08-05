/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Code based on kis_my_paint_shade_selector.cpp from Advanced Color Selector,
 * which in turn is based on "lib/colorchanger_crossed_bowl.hpp" from MyPaint (mypaint.org),
 *
 * SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 * SPDX-FileCopyrightText: 2008 Martin Renold <martinxyz@gmx.ch>
 * SPDX-FileCopyrightText: 2009 Ilya Portnov <nomail>
 */

#include "WGMyPaintShadeSelector.h"

#include <kis_display_color_converter.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_sequential_iterator.h>

#include <QMouseEvent>
#include <QPainter>
#include <QVector4D>

#include <cmath>

template<class Iterator>
void setColorWithIterator(Iterator &it, const KoColor &color, const int pixelSize) {
    memcpy(it.rawData(), color.data(), pixelSize);
}

inline int sqr(int x) {
    return x*x;
}

inline qreal sqr2(qreal x) {
    return (x*x + x)*0.5;
}

inline int signedSqr(int x) {
    return (x > 0) ? x*x : -(x*x);
}

WGMyPaintShadeSelector::WGMyPaintShadeSelector(WGSelectorDisplayConfigSP displayConfig, QWidget *parent, UiMode mode)
    : WGSelectorWidgetBase(displayConfig, parent, mode)
{
    recalculateSizeHD();
}

WGMyPaintShadeSelector::~WGMyPaintShadeSelector()
{

}

void WGMyPaintShadeSelector::setModel(KisVisualColorModelSP model)
{
    if (m_model) {
        disconnect(m_model.data());
        m_model->disconnect(this);
    }
    m_model = model;
    connect(this, SIGNAL(sigChannelValuesChanged(QVector4D)),
            m_model.data(), SLOT(slotSetChannelValues(QVector4D)));
    connect(m_model.data(), SIGNAL(sigChannelValuesChanged(QVector4D,quint32)),
            this, SLOT(slotSetChannelValues(QVector4D)));
    if (m_model->isHSXModel()) {
        slotSetChannelValues(m_model->channelValues());
    }
}

void WGMyPaintShadeSelector::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT sigColorInteraction(true);
        pickColorAt(event->localPos());
    } else {
        event->ignore();
    }
}

void WGMyPaintShadeSelector::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (rect().contains(event->pos())) {
            pickColorAt(event->localPos());
        }
    } else {
        event->ignore();
    }
}

void WGMyPaintShadeSelector::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT sigColorInteraction(false);
    } else {
        event->ignore();
    }
}

void WGMyPaintShadeSelector::paintEvent(QPaintEvent *)
{
    // Hint to the casual reader: some of the calculation here do not
    // what Martin Renold originally intended. Not everything here will make sense.
    // It does not matter in the end, as long as the result looks good.
    if (!m_model || !m_model->isHSXModel()) {
        return;
    }

    // This selector was ported from MyPaint in 2010
    if (!m_realPixelCache || m_realPixelCache->colorSpace() != m_model->colorSpace()) {
        m_realPixelCache = new KisPaintDevice(m_model->colorSpace());
        m_realCircleBorder = new KisPaintDevice(m_model->colorSpace());
//        m_cachedColorSpace = colorSpace();
    }
    else {
        m_realPixelCache->clear();
        m_realCircleBorder->clear();
    }

    const int pixelSize = m_model->colorSpace()->pixelSize();

    QRect pickRectHighDPI = QRect(QPoint(0, 0), size()*devicePixelRatioF());
    KisSequentialIterator it(m_realPixelCache, pickRectHighDPI);
    KisSequentialIterator borderIt(m_realCircleBorder, pickRectHighDPI);
    QVector4D values;
    QVector4D values2;

    while (it.nextPixel() && borderIt.nextPixel()) {
        const int x = it.x();
        const int y = it.y();

        bool needsBlending = getChannelValues(QPoint(x, y), values, values2);

        if (needsBlending) {
            const qreal aaFactor = static_cast<qreal>(values2[3]);
            KoColor color = m_model->convertChannelValuesToKoColor(values2);
            color.setOpacity(aaFactor);
            setColorWithIterator(borderIt, color, pixelSize);
        }

        KoColor color = m_model->convertChannelValuesToKoColor(values);
        setColorWithIterator(it, color, pixelSize);
    }

    KisPainter gc(m_realPixelCache);
    gc.bitBlt(QPoint(0,0), m_realCircleBorder, QRect(rect().topLeft(), rect().size()*devicePixelRatioF()));

    QPainter painter(this);
    QImage renderedImage = displayConverter()->toQImage(m_realPixelCache, displayConfiguration()->previewInPaintingCS());
    renderedImage.setDevicePixelRatio(devicePixelRatioF());

    painter.drawImage(0, 0, renderedImage);
}

void WGMyPaintShadeSelector::resizeEvent(QResizeEvent *event)
{
    WGSelectorWidgetBase::resizeEvent(event);
    recalculateSizeHD();
}

bool WGMyPaintShadeSelector::getChannelValues(QPoint pos, QVector4D &values, QVector4D &blendValues)
{
    bool needsBlending = false;

    const float v_factor = 0.6f;
    const float s_factor = 0.6f;
    const float v_factor2 = 0.013f;
    const float s_factor2 = 0.013f;

    const int stripe_width = (15 * m_sizeHD)/255;
    int s_radiusHD = m_sizeHD/2.6;

    float h = 0;
    float s = 0;
    float v = 0;

    int dx = pos.x() - m_widthHD/2;
    int dy = pos.y() - m_heightHD/2;
    int diag = sqrt(2.0) * m_sizeHD/2;

    int dxs, dys;
    if (dx > 0)
        dxs = dx - stripe_width;
    else
        dxs = dx + stripe_width;
    if (dy > 0)
        dys = dy - stripe_width;
    else
        dys = dy + stripe_width;

    qreal r = std::sqrt(qreal(sqr(dxs)+sqr(dys)));

    if (qMin(abs(dx), abs(dy)) < stripe_width) {
        // horizontal and vertical lines
        bool horizontal = std::abs(dx) > std::abs(dy);
        dx = (dx/qreal(m_sizeHD))*255;
        dy = (dy/qreal(m_sizeHD))*255;

        h = 0;
        // x-axis = value, y-axis = saturation
        v =    dx*v_factor + signedSqr(dx)*v_factor2;
        s = - (dy*s_factor + signedSqr(dy)*s_factor2);
        // but not both at once
        if (horizontal) {
            // horizontal stripe
            s = 0.0;
        } else {
            // vertical stripe
            v = 0.0;
        }
    }
    else if (std::min(std::abs(dx - dy), std::abs(dx + dy)) < stripe_width) {

        dx = (dx/qreal(m_sizeHD))*255;
        dy = (dy/qreal(m_sizeHD))*255;

        h = 0;
        // x-axis = value, y-axis = saturation
        v =    dx*v_factor + signedSqr(dx)*v_factor2;
        s = - (dy*s_factor + signedSqr(dy)*s_factor2);
        // both at once
    }
    else if (r < s_radiusHD+1) {

        // hue
        if (dx > 0)
            h = 90*sqr2(r/s_radiusHD);
        else
            h = 360 - 90*sqr2(r/s_radiusHD);
        s = 256*(atan2f(std::abs(dxs),dys)/M_PI) - 128;

        if (r > s_radiusHD) {
            needsBlending = true;
            // antialiasing boarder
            qreal aaFactor = r-floor(r); // part after the decimal point
            aaFactor = 1-aaFactor;

            qreal fh = m_colorH + h/360.0;
            qreal fs = m_colorS + s/255.0;
            qreal fv = m_colorV + v/255.0;

            fh -= floor(fh);
            fs = qBound(qreal(0.0), fs, qreal(1.0));
            fv = qBound(qreal(0.01), fv, qreal(1.0));
            blendValues = QVector4D(fh, fs, fv, aaFactor);

            h = 180 + 180*atan2f(dys,-dxs)/M_PI;
            v = 255*(r-s_radiusHD)/(diag-s_radiusHD) - 128;
            s = 0; // overwrite the s value that was meant for the inside of the circle
            // here we already have drawn the inside, and the value left should be just the background value
        }
    }
    else {
        // background (hue+darkness gradient)
        h = 180 + 180*atan2f(dys,-dxs)/M_PI;
        v = 255*(r-s_radiusHD)/(diag-s_radiusHD) - 128;
    }

    qreal fh = m_colorH + h/360.0;
    qreal fs = m_colorS + s/255.0;
    qreal fv = m_colorV + v/255.0;

    fh -= floor(fh);
    fs = qBound(qreal(0.0), fs, qreal(1.0));
    fv = qBound(qreal(0.01), fv, qreal(1.0));
    values = QVector4D(fh, fs, fv, 0);

    return needsBlending;
}

void WGMyPaintShadeSelector::pickColorAt(const QPointF &posF)
{
    QPoint pos = (posF * devicePixelRatioF()).toPoint();
    QVector4D values, dummy;
    getChannelValues(pos, values, dummy);
    m_allowUpdates = false;
    Q_EMIT sigChannelValuesChanged(values);
    m_allowUpdates = true;
}

void WGMyPaintShadeSelector::recalculateSizeHD()
{
    m_widthHD = qMax(1, width()) * devicePixelRatioF();
    m_heightHD = qMax(1, height()) *devicePixelRatioF();
    m_sizeHD = qMin(m_widthHD, m_heightHD);
}

void WGMyPaintShadeSelector::slotSetChannelValues(const QVector4D &values)
{
    if (m_allowUpdates) {
        m_colorH = values.x();
        m_colorS = values.y();
        m_colorV = values.z();
        update();
    }
}
