/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QWindow>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QPolygon>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QStyle>
#include <QApplication>
#include <QStyleOptionToolButton>

#include "kis_global.h"
#include "kis_debug.h"
#include "krita_utils.h"

#include "KisStopGradientSlider.h"

KisStopGradientSlider::KisStopGradientSlider(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_selectedStop(0)
    , m_drag(0)
{
    QLinearGradient defaultGradient;
    m_defaultGradient = KoStopGradient::fromQGradient(&defaultGradient);

    setGradientResource(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    setMouseTracking(true);

    QWindow *window = this->window()->windowHandle();
    if (window) {
        connect(window, SIGNAL(screenChanged(QScreen*)), SLOT(updateHandleSize()));
    }
    updateHandleSize();
}

void KisStopGradientSlider::updateHandleSize()
{
    QFontMetrics fm(font());
    const int h = fm.height();
    m_handleSize = QSize(0.34 * h, h);
}

int KisStopGradientSlider::handleClickTolerance() const
{
    // the size of the default text!
    return m_handleSize.width();
}

void KisStopGradientSlider::setGradientResource(KoStopGradientSP gradient)
{
    m_gradient = gradient ? gradient : m_defaultGradient;

    if (m_gradient && m_selectedStop >= 0) {
        m_selectedStop = qBound(0, m_selectedStop, m_gradient->stops().size() - 1);
        emit sigSelectedStop(m_selectedStop);
    } else {
        m_selectedStop = -1;
    }

}

void KisStopGradientSlider::paintHandle(qreal position, const QColor &color, bool isSelected, QString text, QPainter *painter)
{
    const QRect handlesRect = this->handlesStripeRect();

    const int handleCenter = handlesRect.left() + position * handlesRect.width();
    const int handlesHalfWidth = handlesRect.height() * 0.26; // = 1.0 / 0.66 * 0.34 / 2.0 <-- golden ratio

    QPolygon triangle(3);
    triangle[0] = QPoint(handleCenter, handlesRect.top());
    triangle[1] = QPoint(handleCenter - handlesHalfWidth, handlesRect.bottom());
    triangle[2] = QPoint(handleCenter + handlesHalfWidth, handlesRect.bottom());

    QPoint textPos(handleCenter - handlesHalfWidth, handlesRect.top() - handlesRect.height() / 2);

    const qreal lineWidth = 1.0;

    if (!isSelected) {
        painter->setPen(QPen(palette().text(), lineWidth));
        painter->setBrush(QBrush(color));
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawPolygon(triangle);
    } else {
        painter->setPen(QPen(palette().highlight(), 1.5 * lineWidth));
        painter->setBrush(QBrush(color));
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawPolygon(triangle);
    }
    painter->drawText(textPos, text);
}

void KisStopGradientSlider::paintEvent(QPaintEvent* pe)
{
    QWidget::paintEvent(pe);
    QPainter painter(this);
    painter.setPen(Qt::black);

    const QRect previewRect = gradientStripeRect();
    KritaUtils::renderExactRect(&painter, kisGrowRect(previewRect, 1));

    painter.drawRect(previewRect);
    if (m_gradient) {
        QImage image = m_gradient->generatePreview(previewRect.width(), previewRect.height());
        if (!image.isNull()) {
            painter.drawImage(previewRect.topLeft(), image);
        }

        QList<KoGradientStop> handlePositions = m_gradient->stops();
        for (int i = 0; i < handlePositions.count(); i++) {
            QString text = "";
            if (handlePositions[i].type == FOREGROUNDSTOP) {
                text = "FG";
            } else if (handlePositions[i].type == BACKGROUNDSTOP) {
                text = "BG";
            }
            paintHandle(handlePositions[i].position,
                        handlePositions[i].color.toQColor(),
                        i == m_selectedStop, text, &painter);
        }
    }
}

int findNearestHandle(qreal t, const qreal tolerance, const QList<KoGradientStop> &stops)
{
    int result = -1;
    qreal minDistance = tolerance;

    for (int i = 0; i < stops.size(); i++) {
        const KoGradientStop &stop = stops[i];

        const qreal distance = qAbs(t - stop.position);
        if (distance < minDistance) {
            minDistance = distance;
            result = i;
        }
    }

    return result;
}


void KisStopGradientSlider::mousePressEvent(QMouseEvent * e)
{
    if (!allowedClickRegion(handleClickTolerance()).contains(e->pos())) {
        QWidget::mousePressEvent(e);
        return;
    }

    if (e->buttons() != Qt::LeftButton ) {
        QWidget::mousePressEvent(e);
        return;
    }

    const QRect handlesRect = this->handlesStripeRect();
    const qreal t = (qreal(e->x()) - handlesRect.x()) / handlesRect.width();
    const QList<KoGradientStop> stops = m_gradient->stops();

    const int clickedStop = findNearestHandle(t, qreal(handleClickTolerance()) / handlesRect.width(), stops);

    if (clickedStop >= 0) {
        if (m_selectedStop != clickedStop) {
            m_selectedStop = clickedStop;
            emit sigSelectedStop(m_selectedStop);
        }
        m_drag = true;
    } else {
        insertStop(qBound(0.0, t, 1.0));
        m_drag = true;
    }

    update();
    updateCursor(e->pos());
}

void KisStopGradientSlider::mouseReleaseEvent(QMouseEvent * e)
{
     Q_UNUSED(e);
     m_drag = false;
     updateCursor(e->pos());
}

int getNewInsertPosition(const KoGradientStop &stop, const QList<KoGradientStop> &stops)
{
    int result = 0;

    for (int i = 0; i < stops.size(); i++) {
        if (stop.position <= stops[i].position) break;

        result = i + 1;
    }

    return result;
}

void KisStopGradientSlider::mouseMoveEvent(QMouseEvent * e)
{
    updateCursor(e->pos());

    if (m_drag) {
        const QRect handlesRect = this->handlesStripeRect();
        double t = (qreal(e->x()) - handlesRect.x()) / handlesRect.width();

        QList<KoGradientStop> stops = m_gradient->stops();

        if (t < -0.1 || t > 1.1) {
            if (stops.size() > 2 && m_selectedStop >= 0) {
                m_removedStop = stops[m_selectedStop];
                stops.removeAt(m_selectedStop);
                m_selectedStop = -1;
            }
        } else {
            if (m_selectedStop < 0) {
                m_removedStop.position = qBound(0.0, t, 1.0);
                const int newPos = getNewInsertPosition(m_removedStop, stops);
                stops.insert(newPos, m_removedStop);
                m_selectedStop = newPos;
            } else {
                KoGradientStop draggedStop = stops[m_selectedStop];
                draggedStop.position = qBound(0.0, t, 1.0);

                stops.removeAt(m_selectedStop);
                const int newPos = getNewInsertPosition(draggedStop, stops);
                stops.insert(newPos, draggedStop);
                m_selectedStop = newPos;
            }
        }

        m_gradient->setStops(stops);
        emit sigSelectedStop(m_selectedStop);

        update();

    } else {
        QWidget::mouseMoveEvent(e);
    }


}

void KisStopGradientSlider::updateCursor(const QPoint &pos)
{
    const bool isInAllowedRegion =
            allowedClickRegion(handleClickTolerance()).contains(pos);

    QCursor currentCursor;

    if (isInAllowedRegion) {
        const QRect handlesRect = this->handlesStripeRect();
        const qreal t = (qreal(pos.x()) - handlesRect.x()) / handlesRect.width();
        const QList<KoGradientStop> stops = m_gradient->stops();

        const int clickedStop = findNearestHandle(t, qreal(handleClickTolerance()) / handlesRect.width(), stops);

        if (clickedStop >= 0) {
            currentCursor = m_drag ? Qt::ClosedHandCursor : Qt::OpenHandCursor;
        }
    }

    if (currentCursor.shape() != Qt::ArrowCursor) {
        setCursor(currentCursor);
    } else {
        unsetCursor();
    }
}

void KisStopGradientSlider::insertStop(double t)
{
    KIS_ASSERT_RECOVER(t >= 0 && t <= 1.0 ) {
        t = qBound(0.0, t, 1.0);
    }

    QList<KoGradientStop> stops = m_gradient->stops();

    KoColor color;
    m_gradient->colorAt(color, t);

    const KoGradientStop stop(t, color, COLORSTOP);
    const int newPos = getNewInsertPosition(stop, stops);

    stops.insert(newPos, stop);
    m_gradient->setStops(stops);

    m_selectedStop = newPos;
    emit sigSelectedStop(m_selectedStop);
}

QRect KisStopGradientSlider::sliderRect() const
{
    return QRect(QPoint(), size()).adjusted(m_handleSize.width(), 1, -m_handleSize.width(), -1);
}

QRect KisStopGradientSlider::gradientStripeRect() const
{
    const QRect rc = sliderRect();
    return rc.adjusted(0, 0, 0, -m_handleSize.height());
}

QRect KisStopGradientSlider::handlesStripeRect() const
{
    const QRect rc = sliderRect();
    return rc.adjusted(0, rc.height() - m_handleSize.height(), 0, 0);
}

QRegion KisStopGradientSlider::allowedClickRegion(int tolerance) const
{
    QRegion result;
    result += sliderRect();
    result += handlesStripeRect().adjusted(-tolerance, 0, tolerance, 0);
    return result;
}

int KisStopGradientSlider::selectedStop()
{
    return m_selectedStop;
}

void KisStopGradientSlider::setSelectedStop(int selected)
{
    m_selectedStop = selected;
    emit sigSelectedStop(m_selectedStop);

    update();
}

int KisStopGradientSlider::minimalHeight() const
{
    QFontMetrics fm(font());
    const int h = fm.height();

    QStyleOptionToolButton opt;
    QSize sz = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(h, h), this);

    return sz.height() + m_handleSize.height();
}

QSize KisStopGradientSlider::sizeHint() const
{
    const int h = minimalHeight();
    return QSize(2 * h, h);
}

QSize KisStopGradientSlider::minimumSizeHint() const
{
    const int h = minimalHeight();
    return QSize(h, h);
}



