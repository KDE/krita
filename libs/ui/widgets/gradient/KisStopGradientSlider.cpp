/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QWindow>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QPolygon>
#include <QFontMetrics>
#include <QStyle>
#include <QApplication>
#include <QStyleOptionToolButton>
#include <QPainterPath>
#include <QColorDialog>

#include <KisGradientWidgetsUtils.h>
#include "kis_global.h"
#include "kis_debug.h"
#include "krita_utils.h"
#include <KoColor.h>
#include <KisDlgInternalColorSelector.h>

#include "KisStopGradientSlider.h"

KisStopGradientSlider::KisStopGradientSlider(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_selectedStop(0)
    , m_hoveredStop(-1)
    , m_drag(0)
    , m_updateCompressor(40, KisSignalCompressor::FIRST_ACTIVE)
{
    QLinearGradient defaultGradient;
    m_defaultGradient = KoStopGradient::fromQGradient(&defaultGradient);

    setGradientResource(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);

    connect(this, SIGNAL(updateRequested()), &m_updateCompressor, SLOT(start()));
    connect(&m_updateCompressor, SIGNAL(timeout()), this, SLOT(update()));

    QWindow *window = this->window()->windowHandle();
    if (window) {
        connect(window, SIGNAL(screenChanged(QScreen*)), SLOT(updateHandleSize()));
    }
    updateHandleSize();
}

void KisStopGradientSlider::updateHandleSize()
{
    QFontMetrics fm(font());
    const int h = qMax(15, static_cast<int>(std::ceil(fm.height() * 0.75)));
    m_handleSize = QSize(h * 0.75, h);
}

int KisStopGradientSlider::handleClickTolerance() const
{
    // the size of the default text!
    return m_handleSize.width();
}

void KisStopGradientSlider::setGradientResource(KoStopGradientSP gradient)
{
    m_gradient = gradient ? gradient : m_defaultGradient;

    if (m_gradient) {
        m_selectedStop = 0;
    } else {
        m_selectedStop = -1;
    }
    emit sigSelectedStop(m_selectedStop);
}

void KisStopGradientSlider::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    const QRect previewRect = gradientStripeRect();

    if (m_gradient) {
        // Gradient
        KisGradientWidgetsUtils::paintGradientBox(painter, m_gradient, previewRect);
        // Stops
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRect handlesRect = this->handlesStripeRect();
        const bool hasFocus = this->hasFocus();
        const QColor highlightColor = palette().color(QPalette::Highlight);
        const QList<KoGradientStop> handlePositions = m_gradient->stops();
        for (int i = 0; i < handlePositions.count(); ++i) {
            if (i == m_selectedStop) {
                continue;
            }
            KisGradientWidgetsUtils::ColorType colorType;
            if (handlePositions[i].type == FOREGROUNDSTOP) {
                colorType = KisGradientWidgetsUtils::Foreground;
            } else if (handlePositions[i].type == BACKGROUNDSTOP) {
                colorType = KisGradientWidgetsUtils::Background;
            } else {
                colorType = KisGradientWidgetsUtils::Custom;
            }
            KisGradientWidgetsUtils::paintStopHandle(
                painter,
                QPointF(handlesRect.left() + handlePositions[i].position * handlesRect.width(), handlesRect.top()),
                QSizeF(m_handleSize),
                false, i == m_hoveredStop, hasFocus,
                highlightColor,
                { colorType, handlePositions[i].color.toQColor() }
            );
        }
        if (handlePositions.count() > 0 && m_selectedStop >= 0 && m_selectedStop < handlePositions.count()) {
            KisGradientWidgetsUtils::ColorType colorType;
            if (handlePositions[m_selectedStop].type == FOREGROUNDSTOP) {
                colorType = KisGradientWidgetsUtils::Foreground;
            } else if (handlePositions[m_selectedStop].type == BACKGROUNDSTOP) {
                colorType = KisGradientWidgetsUtils::Background;
            } else {
                colorType = KisGradientWidgetsUtils::Custom;
            }
            KisGradientWidgetsUtils::paintStopHandle(
                painter,
                QPointF(handlesRect.left() + handlePositions[m_selectedStop].position * handlesRect.width(), handlesRect.top()),
                QSizeF(m_handleSize),
                true, false, hasFocus,
                highlightColor,
                { colorType, handlePositions[m_selectedStop].color.toQColor() }
            );
        }
    } else {
        painter.setPen(palette().color(QPalette::Mid));
        painter.drawRect(previewRect);
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


void KisStopGradientSlider::mousePressEvent(QMouseEvent *e)
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

    updateHoveredStop(e->pos());
    emit updateRequested();
}

void KisStopGradientSlider::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    m_drag = false;
    int previousHoveredStop = m_hoveredStop;
    updateHoveredStop(e->pos());
    if (previousHoveredStop != m_hoveredStop) {
        emit updateRequested();
    }
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

void KisStopGradientSlider::mouseMoveEvent(QMouseEvent *e)
{
    int previousHoveredStop = m_hoveredStop;
    updateHoveredStop(e->pos());

    if (m_drag) {
        QList<KoGradientStop> stops = m_gradient->stops();
        const QRect augmentedRect = kisGrowRect(rect(), removeStopDistance);

        if (stops.size() > 2 && !augmentedRect.contains(e->pos()))
        {
            if (m_selectedStop >= 0) {
                m_removedStop = stops[m_selectedStop];
                stops.removeAt(m_selectedStop);
                m_selectedStop = -1;
            }
        } else {
            const QRect handlesRect = this->handlesStripeRect();
            double t = qreal(e->pos().x() - handlesRect.left()) / handlesRect.width();
            if (m_selectedStop < 0) {
                if (augmentedRect.contains(e->pos())) {
                    m_removedStop.position = qBound(0.0, t, 1.0);
                    const int newPos = getNewInsertPosition(m_removedStop, stops);
                    stops.insert(newPos, m_removedStop);
                    m_selectedStop = newPos;
                } else {
                    return;
                }
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
        
        emit updateRequested();
    } else {
        if (previousHoveredStop != m_hoveredStop) {
            emit updateRequested();
        }
        QWidget::mouseMoveEvent(e);
    }
}

void KisStopGradientSlider::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        QWidget::mouseDoubleClickEvent(e);
        return;
    }

    const QRect handlesRect = this->handlesStripeRect();
    const qreal t = (qreal(e->x()) - handlesRect.x()) / handlesRect.width();
    const QList<KoGradientStop> stops = m_gradient->stops();
    
    if (qAbs(t - stops[m_selectedStop].position) < qreal(handleClickTolerance()) / handlesRect.width()) {
        chooseSelectedStopColor();
    }
}

void KisStopGradientSlider::handleIncrementInput(int direction, Qt::KeyboardModifiers modifiers)
{
    if (direction == 0) {
        return;
    }
    QList<KoGradientStop> stops = m_gradient->stops();
    if (modifiers & Qt::ControlModifier) {
        m_selectedStop += direction < 0 ? -1 : 1;
        m_selectedStop = qBound(0, m_selectedStop, stops.count() - 1);
    } else if (m_selectedStop >= 0 && m_selectedStop < stops.count()) {
        const qreal increment = modifiers & Qt::ShiftModifier ? 0.001 : 0.01;
        KoGradientStop draggedStop = stops[m_selectedStop];
        draggedStop.position += direction < 0 ? -increment : increment;
        draggedStop.position = qBound(0.0, draggedStop.position, 1.0);

        stops.removeAt(m_selectedStop);
        const int newPos = getNewInsertPosition(draggedStop, stops);
        stops.insert(newPos, draggedStop);
        m_selectedStop = newPos;
        m_gradient->setStops(stops);
    }
    emit sigSelectedStop(m_selectedStop);
    emit updateRequested();
}

void KisStopGradientSlider::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() != 0) {
        handleIncrementInput(e->angleDelta().y(), e->modifiers());
        e->accept();
    } else {
        QWidget::wheelEvent(e);
    }
}

void KisStopGradientSlider::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Left:
        handleIncrementInput(-1, e->modifiers());
        break;
    case Qt::Key_Right:
        handleIncrementInput(1, e->modifiers());
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        chooseSelectedStopColor();
        break;
    case Qt::Key_Delete:
        deleteSelectedStop();
        break;
    default:
        QWidget::keyPressEvent(e);
        break;
    }
}

void KisStopGradientSlider::leaveEvent(QEvent *e)
{
    m_hoveredStop = -1;
    emit updateRequested();
    QWidget::leaveEvent(e);
}

void KisStopGradientSlider::updateHoveredStop(const QPoint &pos)
{
    const bool isInAllowedRegion =
            allowedClickRegion(handleClickTolerance()).contains(pos);

    if (isInAllowedRegion) {
        const QRect handlesRect = this->handlesStripeRect();
        const qreal t = (qreal(pos.x()) - handlesRect.x()) / handlesRect.width();
        const QList<KoGradientStop> stops = m_gradient->stops();

        m_hoveredStop = m_drag ? -1 : findNearestHandle(t, qreal(handleClickTolerance()) / handlesRect.width(), stops);

    } else {
        m_hoveredStop = -1;
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
    const qreal handleWidthOverTwo = static_cast<qreal>(m_handleSize.width()) / 2.0;
    const int hMargin = static_cast<int>(std::ceil(handleWidthOverTwo)) + 2;
    return rect().adjusted(hMargin, 0, -hMargin, 0);
}

QRect KisStopGradientSlider::gradientStripeRect() const
{
    const QRect rc = sliderRect();
    return rc.adjusted(0, 0, 0, -m_handleSize.height() - 4);
}

QRect KisStopGradientSlider::handlesStripeRect() const
{
    const QRect rc = sliderRect();
    return rc.adjusted(0, rc.height() - (m_handleSize.height() + 2), 0, -2);
}

QRegion KisStopGradientSlider::allowedClickRegion(int tolerance) const
{
    Q_UNUSED(tolerance);
    QRegion result;
    result += rect();
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

    emit updateRequested();
}

void KisStopGradientSlider::selectPreviousStop()
{
    if (m_selectedStop < 0) {
        setSelectedStop(0);
    } else if (m_selectedStop > 0) {
        setSelectedStop(m_selectedStop - 1);
    }
}

void KisStopGradientSlider::selectNextStop()
{
    if (m_selectedStop < 0) {
        setSelectedStop(0);
    } else if (m_selectedStop < m_gradient->stops().size() - 1) {
        setSelectedStop(m_selectedStop + 1);
    }
}

void KisStopGradientSlider::deleteSelectedStop(bool selectNeighborStop)
{
    if (m_drag || m_selectedStop < 0) {
        return;
    }

    QList<KoGradientStop> stops = m_gradient->stops();

    if (stops.size() <= 2) {
        return;
    }

    const qreal pos = stops[m_selectedStop].position;
    stops.removeAt(m_selectedStop);
    if (selectNeighborStop) {
        m_selectedStop = findNearestHandle(pos, 2.0, stops);
    } else {
        m_selectedStop = -1;
    }
    m_gradient->setStops(stops);
    emit sigSelectedStop(m_selectedStop);
}

int KisStopGradientSlider::minimalHeight() const
{
    QFontMetrics fm(font());
    const int h = fm.height();

    QStyleOptionToolButton opt;
    QSize sz = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(h, h), this);

    return qMax(32, sz.height()) + m_handleSize.height();
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

void KisStopGradientSlider::chooseSelectedStopColor()
{
    QList<KoGradientStop> stops = m_gradient->stops();
    if (m_selectedStop < 0 || m_selectedStop >= stops.count()) {
        return;
    }

#ifndef Q_OS_MACOS
    KisDlgInternalColorSelector::Config cfg;
    KisDlgInternalColorSelector *dialog = new KisDlgInternalColorSelector(this, stops[m_selectedStop].color, cfg, i18n("Choose a color"));
    dialog->setPreviousColor(stops[m_selectedStop].color);
    auto setColorFn = [dialog, stops, this]() mutable
                      {
                          stops[m_selectedStop].type = COLORSTOP;
                          stops[m_selectedStop].color = dialog->getCurrentColor();
                          m_gradient->setStops(stops);
                          emit sigSelectedStop(m_selectedStop);
                          emit updateRequested();
                      };
    connect(dialog, &KisDlgInternalColorSelector::signalForegroundColorChosen, setColorFn);
#else
    QColorDialog *dialog = new QColorDialog(this);
    dialog->setCurrentColor(stops[m_selectedStop].color.toQColor());
    auto setColorFn = [dialog, stops, this]() mutable
                      {
                          stops[m_selectedStop].type = COLORSTOP;
                          stops[m_selectedStop].color.fromQColor(dialog->currentColor());
                          m_gradient->setStops(stops);
                          emit sigSelectedStop(m_selectedStop);
                          emit updateRequested();
                      };
    connect(dialog, &QColorDialog::currentColorChanged, setColorFn);
#endif
    connect(dialog, &QDialog::accepted, setColorFn);
    connect(dialog, &QDialog::rejected, [stops, this]()
                                        {
                                            m_gradient->setStops(stops);
                                            emit sigSelectedStop(m_selectedStop);
                                            emit updateRequested();
                                        });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}
