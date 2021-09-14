/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimCurvesValuesHeader.h"

#include <math.h>
#include "kis_debug.h"
#include "kis_custom_modifiers_catcher.h"

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <QApplication>
#include <QStyle>

struct KisAnimCurvesValuesHeader::Private
{
    Private()
        : valueOffset(-1.f)
        , scale(1.f)
        , mouseTracking(false)
        , mouseLastPos(QPoint(0,0))
    {}

    qreal valueOffset;
    qreal scale;
    
    bool mouseTracking;
    QPoint mouseLastPos;
    
    QScopedPointer<KisCustomModifiersCatcher> modifiersCatcher;
};

KisAnimCurvesValuesHeader::KisAnimCurvesValuesHeader(QWidget *parent)
    : QHeaderView(Qt::Vertical, parent)
    , m_d(new Private())
{
    m_d->modifiersCatcher.reset(new KisCustomModifiersCatcher(parent));
    m_d->modifiersCatcher->addModifier("pan-zoom", Qt::Key_Space);
}

KisAnimCurvesValuesHeader::~KisAnimCurvesValuesHeader()
{}

void KisAnimCurvesValuesHeader::setScale(qreal scale)
{
    const qreal minimumScale = 0.001f;
    m_d->scale = qMax(scale, minimumScale);
    viewport()->update();
    emit scaleChanged(m_d->scale);
}

qreal KisAnimCurvesValuesHeader::scale() const
{
    return m_d->scale;
}

void KisAnimCurvesValuesHeader::setValueOffset(qreal offset)
{
    m_d->valueOffset = offset;
    viewport()->update();
    valueOffsetChanged(m_d->valueOffset);
}

qreal KisAnimCurvesValuesHeader::valueOffset() const
{
    return m_d->valueOffset;
}

qreal KisAnimCurvesValuesHeader::step() const
{
    const int MIN_PIXEL_PER_STEP = UNIT_SIZE_PIXELS - 8;
    const int MAX_PIXEL_PER_STEP = UNIT_SIZE_PIXELS * 10;

    const qreal valueSpan = visibleValueDifference();
    qreal step = roundDownPower10(valueSpan * 5);

    if (pixelsPerStep(step) < MIN_PIXEL_PER_STEP) {
        step *= 10;
    } else if (pixelsPerStep(step) >= MAX_PIXEL_PER_STEP) {
        step /= 10;
    }

    return step;
}

qreal KisAnimCurvesValuesHeader::valueToWidget(qreal value) const
{
    return rect().height() - (value - valueOffset()) * scaledUnit();
}

qreal KisAnimCurvesValuesHeader::widgetToValue(qreal position) const
{
    return (position - rect().height()) / (scaledUnit() * -1) + valueOffset();
}

qreal KisAnimCurvesValuesHeader::visibleValueDifference() const
{
    return visibleValueMax() - visibleValueMin();
}

void KisAnimCurvesValuesHeader::zoomToFitRange(qreal min, qreal max)
{
    const qreal range = (max-min);
    const qreal rangePixels = range * UNIT_SIZE_PIXELS;
    setValueOffset(min);
    setScale(rect().height() / rangePixels);
}

void KisAnimCurvesValuesHeader::paintEvent(QPaintEvent */*e*/)
{
    QPainter painter(viewport());

    // Colors.
    const QColor textColor = qApp->palette().color(QPalette::ButtonText);
    const QColor majorNotchColor = QColor(textColor.red(), textColor.green(), textColor.blue(), 192);
    const QColor minorNotchColor = QColor(textColor.red(), textColor.green(), textColor.blue(), 128);
    const QColor zeroColor = qApp->palette().highlight().color();

    const qreal valueStep = step();
    const qreal firstVisibleValue = firstVisibleStep();

    const int visibleSteps = visibleValueDifference() / valueStep;
    const int minorNotches = pixelsPerStep(valueStep) >= (UNIT_SIZE_PIXELS * 2) ? 9 : 3;
    const int majorNotchLength = rect().width();
    const int minorNotchLength = 12;

    // Draw notch at each major step.
    //const qreal upperValueThreshold = firstVisibleValue + (visibleSteps + 2) * valueStep;
    for (int major = 0; major <= visibleSteps + 1; major++) {
        const qreal value = firstVisibleValue + valueStep * major;

        const QPoint majorRight = QPoint(majorNotchLength, valueToWidget(value));
        const QPoint majorLeft = QPoint(0, valueToWidget(value));

        painter.setPen(value != 0 ? majorNotchColor : zeroColor);
        painter.drawLine(majorLeft, majorRight);

        // Draw interior notches at minor substeps.
        const qreal pixelsPerMinorNotch = pixelsPerStep(valueStep) / (minorNotches + 1);
        for (int minor = 0; minor < minorNotches; minor++) {
            const QPoint minorRight = QPoint(rect().width(), majorRight.y() + pixelsPerMinorNotch * (minor + 1));
            const QPoint minorLeft = QPoint(minorRight.x() - minorNotchLength, minorRight.y());

            painter.setPen(minorNotchColor);
            painter.drawLine(minorLeft, minorRight);
        }

        {   // Draw label.
            const int padding = 4;

            const QString label = QString::number(value, 'f', valueStep < 1 ? 2 : 0);
            const QRect textRect = QRect(0, majorLeft.y(), rect().width() - minorNotchLength - padding, 32);

            painter.setPen(value != 0 ? textColor : zeroColor);
            painter.drawText(textRect, label, QTextOption(Qt::AlignRight));
        }
    }
}

void KisAnimCurvesValuesHeader::mouseMoveEvent(QMouseEvent* mouseEvent) {
    if (mouseEvent->buttons() & Qt::LeftButton) {
        if (m_d->mouseTracking) {
            const qreal oldValue = orientation() == Qt::Vertical ? m_d->mouseLastPos.y() : m_d->mouseLastPos.x();
            const qreal newValue = orientation() == Qt::Vertical ? mouseEvent->pos().y() : mouseEvent->pos().x();
            if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
                const qreal delta = (newValue - oldValue);
                setValueOffset(valueOffset() + delta * step() / 64);
            } else {
                const qreal delta = -1 * (newValue - oldValue) / 16;
                setScale(scale() + delta / step());
            }
            m_d->mouseLastPos = mouseEvent->pos();
        }
    } else {
        if (m_d->mouseTracking == true) {
            m_d->mouseTracking = false;
        }
    }
    
    QHeaderView::mouseMoveEvent(mouseEvent);
}

void KisAnimCurvesValuesHeader::mousePressEvent(QMouseEvent* mouseEvent) {
    if (mouseEvent->buttons() & Qt::LeftButton) {
        m_d->mouseTracking = true;
        m_d->mouseLastPos = mouseEvent->pos();
    }
    
    QHeaderView::mousePressEvent(mouseEvent);
}
