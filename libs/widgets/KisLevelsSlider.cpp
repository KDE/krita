/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainter>
#include <QMouseEvent>

#include <algorithm>
#include <cmath>

#include <kis_painting_tweaks.h>

#include "KisLevelsSlider.h"

KisLevelsSlider::KisLevelsSlider(QWidget *parent)
    : QWidget(parent)
    , m_constrainPositions(true)
    , m_selectedHandle(-1)
    , m_hoveredHandle(-1)
{
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);
}

KisLevelsSlider::~KisLevelsSlider()
{}

qreal KisLevelsSlider::handlePosition(int handleIndex) const
{
    Q_ASSERT(handleIndex >= 0 && handleIndex < m_handles.size());

    return m_handles[handleIndex].position;
}

QColor KisLevelsSlider::handleColor(int handleIndex) const
{
    Q_ASSERT(handleIndex >= 0 && handleIndex < m_handles.size());

    return m_handles[handleIndex].color;
}

void KisLevelsSlider::setHandlePosition(int handleIndex, qreal newPosition)
{
    Q_ASSERT(handleIndex >= 0 && handleIndex < m_handles.size());

    if (m_constrainPositions) {
        newPosition =
            qBound(
                handleIndex == m_handles.first().index ? 0.0 : m_handles[handleIndex - 1].position + minimumSpaceBetweenHandles,
                newPosition,
                handleIndex == m_handles.last().index ? 1.0 : m_handles[handleIndex + 1].position - minimumSpaceBetweenHandles
            );
    } else {
        newPosition = qBound(0.0, newPosition, 1.0);
    }

    if (newPosition == m_handles[handleIndex].position) {
        return;
    }

    m_handles[handleIndex].position = newPosition;

    update();
    emit handlePositionChanged(handleIndex, newPosition);
}

void KisLevelsSlider::setHandleColor(int handleIndex, const QColor &newColor)
{
    Q_ASSERT(handleIndex >= 0 && handleIndex < m_handles.size());

    if (newColor == m_handles[handleIndex].color) {
        return;
    }

    m_handles[handleIndex].color = newColor;
    
    update();
    emit handleColorChanged(handleIndex, newColor);
}

QSize KisLevelsSlider::sizeHint() const
{
    return QSize(256, 20) + QSize(handleWidth, handleHeight);
}

QSize KisLevelsSlider::minimumSizeHint() const
{
    return QSize(128, 20) + QSize(handleWidth, handleHeight);
}

QRect KisLevelsSlider::gradientRect() const
{
    const int margin = handleWidth / 2;
    return rect().adjusted(margin, 0, -margin, -handleHeight);
}

QVector<KisLevelsSlider::Handle> KisLevelsSlider::sortedHandles() const
{
    QVector<Handle> sortedHandles_ = m_handles;
    std::sort(sortedHandles_.begin(), sortedHandles_.end(),
        [](const Handle &lhs, const Handle &rhs)
        {
            return qFuzzyCompare(lhs.position, rhs.position) ? lhs.index < rhs.index : lhs.position < rhs.position;
        }
    );
    return sortedHandles_;
}

int KisLevelsSlider::closestHandleToPosition(qreal position) const
{
    const QVector<Handle> sortedHandles_ = sortedHandles();
    int handleIndex = -1;

    if (position <= sortedHandles_.first().position) {
        handleIndex = sortedHandles_.first().index;
    } else if (position >= sortedHandles_.last().position) {
        handleIndex = sortedHandles_.last().index;
    } else {
        for (int i = 0; i < sortedHandles_.size() - 1; ++i) {
            if (position >= sortedHandles_[i + 1].position) {
                continue;
            }
            const qreal middlePoint = (sortedHandles_[i].position + sortedHandles_[i + 1].position) / 2.0;
            handleIndex = position <= middlePoint ? sortedHandles_[i].index : sortedHandles_[i + 1].index;
            break;
        }
    }

    return handleIndex;
}

qreal KisLevelsSlider::positionFromX(int x) const
{
    const QRect gradientRect_ = gradientRect();
    return static_cast<qreal>(x - gradientRect_.left()) / static_cast<qreal>(gradientRect_.width());
}

int KisLevelsSlider::closestHandleToX(int x) const
{
    return closestHandleToPosition(positionFromX(x));
}

int KisLevelsSlider::xFromPosition(qreal position) const
{
    const QRect gradientRect_ = gradientRect();
    return static_cast<int>(position * static_cast<qreal>(gradientRect_.width())) + gradientRect_.left();
}

void KisLevelsSlider::handleIncrementInput(int direction, Qt::KeyboardModifiers modifiers)
{
    if (direction == 0) {
        return;
    }
    if (modifiers & Qt::ControlModifier) {
        m_selectedHandle += direction < 0 ? -1 : 1;
        m_selectedHandle = qBound(0, m_selectedHandle, m_handles.size() - 1);
        update();
    } else if (m_selectedHandle >= 0 && m_selectedHandle < m_handles.size()) {
        const qreal increment = modifiers & Qt::ShiftModifier ? slowPositionIncrement : normalPositionIncrement;
        const qreal position = m_handles[m_selectedHandle].position + (direction < 0 ? -increment : increment);
        setHandlePosition(m_selectedHandle, position);
    }
}

void KisLevelsSlider::paintHandle(QPainter &painter, const QRect &rect, const Handle &handle)
{
    painter.setRenderHint(QPainter::Antialiasing, false);
    const int halfHandleWidth = handleWidth / 2.0;
    {
        const QPolygon shape({
            {rect.left() + halfHandleWidth, rect.top()},
            {rect.right() + 1, rect.top() + halfHandleWidth},
            {rect.right() + 1, rect.bottom() + 1},
            {rect.left(), rect.bottom() + 1},
            {rect.left(), rect.top() + halfHandleWidth}
        });
        painter.setPen(Qt::NoPen);
        const bool isSelected = handle.index == m_selectedHandle;
        QColor brushColor =
            isSelected && hasFocus()
            ? KisPaintingTweaks::blendColors(handle.color, palette().highlight().color(), 0.25)
            : handle.color;
        if (!isEnabled()) {
            brushColor.setAlpha(64);
        }
        painter.setBrush(brushColor);
        painter.drawPolygon(shape);
    }
    {
        const QPolygon shape({
            {rect.left() + halfHandleWidth, rect.top()},
            {rect.right(), rect.top() + halfHandleWidth},
            {rect.right(), rect.bottom()},
            {rect.left(), rect.bottom()},
            {rect.left(), rect.top() + halfHandleWidth}
        });
        const bool isHovered = handle.index == m_hoveredHandle;
        QColor penColor = isHovered && isEnabled() ? palette().highlight().color() : palette().text().color();
        if (!isHovered) {
            penColor.setAlpha(64);
        }
        painter.setPen(penColor);
        painter.setBrush(Qt::NoBrush);
        painter.drawPolygon(shape);
    }
}

void KisLevelsSlider::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    const QRect gradientRect_ = gradientRect();
    // Gradient
    painter.save();
    paintGradient(painter, gradientRect_);
    painter.restore();
    // Border
    QColor borderColor = palette().text().color();
    borderColor.setAlpha(64);
    painter.setPen(borderColor);
    painter.drawRect(gradientRect_.adjusted(0, 0, -1, -1));
    // Handles
    const QVector<Handle> sortedHandles_ = sortedHandles();
    const int halfHandleWidth = handleWidth / 2;
    painter.save();
    for (const Handle &handle : sortedHandles_) {
        const int handleX = static_cast<int>(qRound(handle.position * static_cast<qreal>(gradientRect_.width() - 1))) + halfHandleWidth;
        const QRect handleRect(handleX - halfHandleWidth, gradientRect_.bottom() + 1, handleWidth, handleHeight);
        paintHandle(painter, handleRect, handle);
    }
    painter.restore();
}

void KisLevelsSlider::mousePressEvent(QMouseEvent *e)
{
    if (m_handles.size() == 0) {
        return;
    }
    if (e->button() != Qt::LeftButton) {
        return;
    }

    qreal mousePosition = positionFromX(e->x());
    int handleIndex = closestHandleToPosition(mousePosition);

    if (handleIndex != -1) {
        m_selectedHandle = handleIndex;
        const int handleX = xFromPosition(m_handles[handleIndex].position);
        if (qAbs(handleX - e->x()) > handleWidth) {
            setHandlePosition(handleIndex, mousePosition);
        } else {
            update();
        }
    }
}

void KisLevelsSlider::mouseMoveEvent(QMouseEvent *e)
{
    if (m_handles.size() == 0) {
        return;
    }

    if (e->buttons() & Qt::LeftButton && m_selectedHandle != -1) {
        setHandlePosition(m_selectedHandle, positionFromX(e->x()));
    } else {
        int handleIndex = closestHandleToX(e->x());
        if (handleIndex != -1) {
            m_hoveredHandle = handleIndex;
            update();
        }
    }

}

void KisLevelsSlider::leaveEvent(QEvent *e)
{
    m_hoveredHandle = -1;
    update();
    QWidget::leaveEvent(e);
}

void KisLevelsSlider::keyPressEvent(QKeyEvent *e)
{
    if (m_handles.size() == 0) {
        return;
    }
    if (m_selectedHandle == -1) {
        return;
    }

    switch (e->key()) {
    case Qt::Key_Left:
        handleIncrementInput(-1, e->modifiers());
        return;
    case Qt::Key_Right:
        handleIncrementInput(1, e->modifiers());
        return;
    default:
        QWidget::keyPressEvent(e);
    }
}

void KisLevelsSlider::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() != 0) {
        handleIncrementInput(e->angleDelta().y(), e->modifiers());
        e->accept();
    } else {
        QWidget::wheelEvent(e);
    }
}


KisInputLevelsSlider::KisInputLevelsSlider(QWidget *parent)
    : KisLevelsSlider(parent)
{
    m_handles.resize(2);
    m_handles[0].index = 0;
    m_handles[0].position = 0.0;
    m_handles[0].color = Qt::black;
    m_handles[1].index = 1;
    m_handles[1].position = 1.0;
    m_handles[1].color = Qt::white;
    m_selectedHandle = 0;
    connect(this, &KisInputLevelsSlider::handlePositionChanged,
        [this](int handleIndex, qreal position)
        {
            if (handleIndex == m_handles.first().index) {
                emit blackPointChanged(position);
            } else if (handleIndex == m_handles.last().index) {
                emit whitePointChanged(position);
            }
        }
    );
}

KisInputLevelsSlider::~KisInputLevelsSlider()
{}

qreal KisInputLevelsSlider::blackPoint() const
{
    return m_handles.first().position;
}

qreal KisInputLevelsSlider::whitePoint() const
{
    return m_handles.last().position;
}

void KisInputLevelsSlider::setBlackPoint(qreal newBlackPoint)
{
    setHandlePosition(m_handles.first().index, newBlackPoint);
}

void KisInputLevelsSlider::setWhitePoint(qreal newWhitePoint)
{
    setHandlePosition(m_handles.last().index, newWhitePoint);
}

void KisInputLevelsSlider::reset(qreal newBlackPoint, qreal newWhitePoint)
{
    newBlackPoint = qBound(0.0, newBlackPoint, 1.0);
    newWhitePoint = qBound(0.0, newWhitePoint, 1.0);

    if (m_constrainPositions) {
        if (newWhitePoint < newBlackPoint + minimumSpaceBetweenHandles) {
            newWhitePoint = newBlackPoint + minimumSpaceBetweenHandles;
            if (newWhitePoint < 1.0) {
                newWhitePoint = 1.0;
                newBlackPoint = 1.0 - minimumSpaceBetweenHandles;
            }
        }
        if (newBlackPoint <= whitePoint() - minimumSpaceBetweenHandles) {
            setBlackPoint(newBlackPoint);
            setWhitePoint(newWhitePoint);
        } else {
            setWhitePoint(newWhitePoint);
            setBlackPoint(newBlackPoint);
        }
    } else {
        setBlackPoint(newBlackPoint);
        setWhitePoint(newWhitePoint);
    }
}

void KisInputLevelsSlider::paintBottomGradientMiddleSection(QImage &gradientImage, const QVector<Handle> &sortedHandles_)
{
    if (m_handles.size() < 2) {
        return;
    }

    const int startPos = static_cast<int>(qRound(sortedHandles_.first().position * static_cast<qreal>(gradientImage.width() - 1))) + 1;
    const int endPos = static_cast<int>(qRound(sortedHandles_.last().position * static_cast<qreal>(gradientImage.width() - 1))) + 1;
    QRgb *pixel = reinterpret_cast<QRgb*>(gradientImage.bits()) + startPos;
    for (int x = startPos; x < endPos; ++x, ++pixel) {
        const qreal t = static_cast<qreal>(x - startPos) / static_cast<qreal>(endPos - startPos);
        *pixel =KisPaintingTweaks::blendColors(
                    sortedHandles_.last().color,
                    sortedHandles_.first().color,
                    t
                ).rgba();
    }
}

void KisInputLevelsSlider::paintGradient(QPainter &painter, const QRect &rect)
{
    if (m_handles.size() == 0) {
        return;
    }

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if (!isEnabled()) {
        painter.setOpacity(0.5);
    }
    QImage gradientImage(rect.width(), 1, QImage::Format_ARGB32);
    const int halfGradientHeight = rect.height() / 2;

    // Top gradient
    {
        QRgb *pixel = reinterpret_cast<QRgb*>(gradientImage.bits());
        for (int x = 0; x < gradientImage.width(); ++x, ++pixel) {
            const qreal t = static_cast<qreal>(x) / static_cast<qreal>(gradientImage.width() - 1);
            *pixel =KisPaintingTweaks::blendColors(m_handles.last().color, m_handles.first().color, t).rgba();
        }
    }
    painter.drawImage(rect.adjusted(0, 0, 0, -(halfGradientHeight + (rect.height() & 1 ? 1 : 0))), gradientImage);

    // Bottom gradient
    QVector<Handle> sortedHandles_ = sortedHandles();
    {
        const int endPos = static_cast<int>(qRound(sortedHandles_.first().position * static_cast<qreal>(gradientImage.width() - 1)) + 1);
        QRgb *pixel = reinterpret_cast<QRgb*>(gradientImage.bits());
        for (int x = 0; x < endPos; ++x, ++pixel) {
            *pixel = sortedHandles_.first().color.rgba();
        }
    }
    paintBottomGradientMiddleSection(gradientImage, sortedHandles_);
    {
        const int startPos = static_cast<int>(qRound(sortedHandles_.last().position * static_cast<qreal>(gradientImage.width() - 1)) + 1);
        QRgb *pixel = reinterpret_cast<QRgb*>(gradientImage.bits()) + startPos;
        for (int x = startPos; x < rect.width(); ++x, ++pixel) {
            *pixel = sortedHandles_.last().color.rgba();
        }
    }
    painter.drawImage(rect.adjusted(0, halfGradientHeight, 0, 0), gradientImage);
}


KisInputLevelsSliderWithGamma::KisInputLevelsSliderWithGamma(QWidget *parent)
    : KisInputLevelsSlider(parent)
    , m_gamma(1.0)
{
    m_handles.last().index = 2;
    m_handles.insert(1, {1, 0.5, Qt::gray});
}

KisInputLevelsSliderWithGamma::~KisInputLevelsSliderWithGamma()
{}

qreal KisInputLevelsSliderWithGamma::gamma() const
{
    return m_gamma;
}

void KisInputLevelsSliderWithGamma::setHandlePosition(int handleIndex, qreal newPosition)
{
    Q_ASSERT(handleIndex >= 0 && handleIndex < m_handles.size());

    if (handleIndex == 1) {
        newPosition = qBound(m_handles.first().position, newPosition, m_handles.last().position);
        if (newPosition == m_handles[1].position) {
            return;
        }
        m_handles[1].position = newPosition;
        m_gamma = positionToGamma();
        update();
        emit handlePositionChanged(1, newPosition);
        emit gammaChanged(m_gamma);
    } else {
        if (handleIndex == m_handles.first().index) {
            newPosition = qBound(0.0, newPosition, m_handles.last().position - minimumSpaceBetweenHandles);
        } else if (handleIndex == m_handles.last().index) {
            newPosition = qBound(m_handles.first().position + minimumSpaceBetweenHandles, newPosition, 1.0);
        }
        if (newPosition == m_handles[handleIndex].position) {
            return;
        }
        m_handles[handleIndex].position = newPosition;
        m_handles[1].position = gammaToPosition();
        update();
        emit handlePositionChanged(handleIndex, newPosition);
        emit handlePositionChanged(1, newPosition);
    }

}

void KisInputLevelsSliderWithGamma::setGamma(qreal newGamma)
{
    newGamma = qBound(0.1, newGamma, 10.0);
    if (newGamma == m_gamma) {
        return;
    }
    m_gamma = newGamma;
    m_handles[1].position = gammaToPosition();
    update();
    emit gammaChanged(m_gamma);
    emit handlePositionChanged(1, m_handles[1].position);
}

void KisInputLevelsSliderWithGamma::reset(qreal newBlackPoint, qreal newWhitePoint)
{
    KisInputLevelsSlider::reset(newBlackPoint, newWhitePoint);
}

void KisInputLevelsSliderWithGamma::reset(qreal newBlackPoint, qreal newWhitePoint, qreal newGamma)
{
    reset(newBlackPoint, newWhitePoint);
    setGamma(newGamma);
}

void KisInputLevelsSliderWithGamma::paintBottomGradientMiddleSection(QImage &gradientImage, const QVector<Handle> &sortedHandles_)
{
    if (m_handles.size() < 2) {
        return;
    }
    if (m_handles.size() < 3) {
        KisInputLevelsSlider::paintBottomGradientMiddleSection(gradientImage, sortedHandles_);
        return;
    }

    const qreal inverseGamma = 1.0 / m_gamma;
    const int startPos = static_cast<int>(qRound(sortedHandles_.first().position * static_cast<qreal>(gradientImage.width() - 1)));
    const int endPos = static_cast<int>(qRound(sortedHandles_.last().position * static_cast<qreal>(gradientImage.width() - 1))) + 1;
    QRgb *pixel = reinterpret_cast<QRgb*>(gradientImage.bits()) + startPos;
    for (int x = startPos; x < endPos; ++x, ++pixel) {
        const qreal t = static_cast<qreal>(x - startPos) / static_cast<qreal>(endPos - startPos);
        *pixel =KisPaintingTweaks::blendColors(
                    sortedHandles_.last().color,
                    sortedHandles_.first().color,
                    std::pow(t, inverseGamma)
                ).rgba();
    }
}

qreal KisInputLevelsSliderWithGamma::gammaToPosition() const
{
    qreal relativePosition;
    const qreal log1_2 = std::log(0.5);
    // the function would be "relativePosition = exp(gamma * ln(1/2)" but since
    // we want to limit the gamma from 0.1 to 10 we map the gamma
    // in a way such that "exp(max_gamma * ln(1/2)) = 1" and
    // "exp(min_gamma * ln(1/2)) = 0"
    if (m_gamma > 1.0) {
        const qreal mappedPosition = std::exp(10.0 * log1_2);
        relativePosition = (std::exp(m_gamma * log1_2) - mappedPosition) / (1.0 - mappedPosition * 2.0);
    } else {
        const qreal mappedPosition = std::exp(0.1 * log1_2);
        relativePosition = (std::exp(m_gamma * log1_2) + mappedPosition - 1.0) / (mappedPosition * 2.0 - 1.0);
    }
    return blackPoint() + relativePosition * (whitePoint() - blackPoint());
}

qreal KisInputLevelsSliderWithGamma::positionToGamma() const
{
    const qreal relativePosition = (handlePosition(1) - blackPoint()) / (whitePoint() - blackPoint());
    const qreal log1_2 = std::log(0.5);
    // the function would be "gamma = ln(relativePosition) / ln(1/2)" but since
    // we want to limit the gamma from 0.1 to 10 we map the relative position
    // in a way such that "ln(min_relativePosition) / ln(1/2) = 10" and
    // "ln(max_relativePosition) / ln(1/2) = 0.1"
    if (relativePosition < 0.5) {
        const qreal mappedPosition = std::exp(10.0 * log1_2);
        return std::log(mappedPosition + relativePosition - relativePosition * mappedPosition * 2.0) / log1_2;
    } else {
        const qreal mappedPosition = std::exp(0.1 * log1_2);
        return std::log(1 - (mappedPosition + relativePosition) + relativePosition * mappedPosition * 2.0) / log1_2;
    }
}


KisOutputLevelsSlider::KisOutputLevelsSlider(QWidget *parent)
    : KisInputLevelsSlider(parent)
{
    m_constrainPositions = false;
}

KisOutputLevelsSlider::~KisOutputLevelsSlider()
{}


KisThresholdSlider::KisThresholdSlider(QWidget *parent)
    : KisInputLevelsSlider(parent)
{
    m_constrainPositions = false;
}

KisThresholdSlider::~KisThresholdSlider()
{}

qreal KisThresholdSlider::threshold() const
{
    return blackPoint();
}

void KisThresholdSlider::setHandlePosition(int handleIndex, qreal newPosition)
{
    Q_ASSERT(handleIndex >= 0 && handleIndex < m_handles.size());
    reset(newPosition, newPosition);
}

void KisThresholdSlider::setBlackPoint(qreal newBlackPoint)
{
    reset(newBlackPoint, newBlackPoint);
}

void KisThresholdSlider::setWhitePoint(qreal newWhitePoint)
{
    reset(newWhitePoint, newWhitePoint);
}

void KisThresholdSlider::reset(qreal newBlackPoint, qreal newWhitePoint)
{
    Q_UNUSED(newWhitePoint);

    newBlackPoint = qBound(0.0, newBlackPoint, 1.0);
    if (newBlackPoint == blackPoint()) {
        return;
    }
    m_handles.first().position = m_handles.last().position = newBlackPoint;
    update();
    emit handlePositionChanged(0, newBlackPoint);
    emit blackPointChanged(newBlackPoint);
    emit handlePositionChanged(1, newBlackPoint);
    emit whitePointChanged(newBlackPoint);
    emit thresholdChanged(newBlackPoint);
}

void KisThresholdSlider::setThreshold(qreal newThreshold)
{
    reset(newThreshold, newThreshold);
}

void KisThresholdSlider::paintBottomGradientMiddleSection(QImage&, const QVector<Handle>&)
{}

void KisThresholdSlider::paintHandle(QPainter &painter, const QRect &rect, const Handle &handle)
{
    if (handle.index != m_handles.first().index) {
        return;
    }
    if (m_hoveredHandle >= 0) {
        m_hoveredHandle = 0;
    }
    if (m_selectedHandle >= 0) {
        m_selectedHandle = 0;
    }
    KisLevelsSlider::paintHandle(painter, rect, handle);
}
