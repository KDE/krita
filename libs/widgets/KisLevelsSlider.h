/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LEVELS_SLIDER_H
#define KIS_LEVELS_SLIDER_H

#include <QWidget>
#include <QVector>

#include "kritawidgets_export.h"

/**
 * @brief A base class for levels slider like widgets: a slider with a gradient
 * and multiple handles
 */
class KRITAWIDGETS_EXPORT KisLevelsSlider : public QWidget
{
    Q_OBJECT

public:
    KisLevelsSlider(QWidget *parent);
    ~KisLevelsSlider();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    /**
     * @brief Gets the normalized position of a given handle
     */
    qreal handlePosition(int handleIndex) const;
    /**
     * @brief Gets the color associated with a given handle
     */
    QColor handleColor(int handleIndex) const;
    /**
     * @brief Gets the rect where the gradient will be painted
     */
    virtual QRect gradientRect() const;

public Q_SLOTS:
    /**
     * @brief Sets the normalized position of the given handle
     */
    virtual void setHandlePosition(int handleIndex, qreal newPosition);
    /**
     * @brief Sets the color associated with the given handle
     */
    virtual void setHandleColor(int handleIndex, const QColor &newColor);

Q_SIGNALS:
    /**
     * @brief Signal emited when the position of a handle changes
     */
    void handlePositionChanged(int handleIndex, qreal position);
    /**
     * @brief Signal emited when the color associated with a handle changes
     */
    void handleColorChanged(int handleIndex, const QColor &color);

protected:
    struct Handle
    {
        int index;
        qreal position;
        QColor color;
    };

    static constexpr int handleWidth{11};
    static constexpr int handleHeight{11};
    static constexpr qreal minimumSpaceBetweenHandles{0.001};
    static constexpr qreal normalPositionIncrement{0.01};
    static constexpr qreal slowPositionIncrement{0.001};

    /**
     * @brief The collection of handles
     */
    QVector<Handle> m_handles;
    /**
     * @brief This variable indicates if the handles can have unordered
     * positions. If it is set to true then the user won't be able to move a
     * handle pass another one. If it is set to false then the ser will be able
     * to move the handles freely
     */
    int m_constrainPositions;

    int m_selectedHandle;
    int m_hoveredHandle;
    
    /**
     * @brief Regardless the index of a handle, they can be unordered in terms
     * of the position. This returns a sorted vector with the handles that have
     * a smaller position first. If two handles have the same position then the
     * index is used for sorting
     */
    QVector<Handle> sortedHandles() const;
    /**
     * @brief Given a normalized position, this function returns the closest
     * handle to that position
     */
    int closestHandleToPosition(qreal position) const;
    /**
     * @brief Given a widget-relative x position in pixels, this function
     * returns the normalized position relative to the gradient rect
     */
    qreal positionFromX(int x) const;
    /**
     * @brief Given a widget-relative x position, this function returns the
     * closest handle to that position
     */
    int closestHandleToX(int x) const;
    /**
     * @brief Given a gradient rect relative position, this function returns the
     * x position in pixels relative to the widget
     */
    int xFromPosition(qreal position) const;
    /**
     * @brief Derived classes must override this function to draw the gradient
     * inside the given rect. A border is automatically drawn after
     */
    virtual void paintGradient(QPainter &painter, const QRect &rect) = 0;
    /**
     * @brief Override this function to paint custom handles
     */
    virtual void paintHandle(QPainter &painter, const QRect &rect, const Handle &handle);

    void handleIncrementInput(int direction, Qt::KeyboardModifiers modifiers);

    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
};

/**
 * @brief This is a simple input levels slider that has no gamma handle. Use it
 * if you want to show a simple mapping or contrast adjustement. The handles
 * are constrained so that the black point handle can not pass the white point
 * handle and viceversa
 */
class KRITAWIDGETS_EXPORT KisInputLevelsSlider : public KisLevelsSlider
{
    Q_OBJECT

public:
    KisInputLevelsSlider(QWidget *parent = nullptr);
    ~KisInputLevelsSlider();

    /**
     * @brief Get the normalized black point
     */
    qreal blackPoint() const;
    /**
     * @brief Get the normalized white point
     */
    qreal whitePoint() const;

public Q_SLOTS:
    /**
     * @brief Sets the black point
     */
    virtual void setBlackPoint(qreal newBlackPoint);
    /**
     * @brief Sets the white point
     */
    virtual void setWhitePoint(qreal newWhitePoint);
    /**
     * @brief Sometimes you want to set the range to a totally different place,
     * but the new black point can be greater than the old white point so the
     * new black point position will be constrained to the old white position.
     * This function allows to set both values at once to prevent that
     * undesirable effect. Use it when the widget has to show new values, for a
     * different set of levels parameters for example
     */
    virtual void reset(qreal newBlackPoint, qreal newWhitePoint);

Q_SIGNALS:
    /**
     * @brief Signal emited when the black point changes
     */
    void blackPointChanged(qreal newBlackPoint);
    /**
     * @brief Signal emited when the white point changes
     */
    void whitePointChanged(qreal newWhitePoint);

protected:
    /**
     * @brief Custom gradient painter. This paints two bars in the gradient
     * rect, one on top of the other. The top one shows a simple black to white
     * (more exactly, color of the first handle to color of the last handle)
     * linear gradient. The bottom one shows black (first handle's color) from
     * the right side of the rect to the black point, and white (last handle's
     * color) from the white point to the left side of the rect. In the middle
     * of the two handles, the "paintBottomGradientMiddleSection" is used to
     * fill the bar
     */
    void paintGradient(QPainter &painter, const QRect &rect) override;
    /**
     * @brief This is used to fill the space between the tho handles in the
     * bottom bar of the "gradient". It just paints a linear gradient that goes
     * from black (first handle's color) to white (last handle's color). Derived
     * classes can override this function if they only want to change that area.
     * "gradientImage" is a 256x1px image.
     */
    virtual void paintBottomGradientMiddleSection(QImage &gradientImage, const QVector<Handle> &sortedHandles_);
};

/**
 * @brief This is a input levels slider that has a gamma handle. The handles
 * are constrained so that the black point handle can not pass the white point
 * handle and viceversa
 */
class KRITAWIDGETS_EXPORT KisInputLevelsSliderWithGamma : public KisInputLevelsSlider
{
    Q_OBJECT

public:
    KisInputLevelsSliderWithGamma(QWidget *parent = nullptr);
    ~KisInputLevelsSliderWithGamma();

    /**
     * @brief Get the gamma value
     */
    qreal gamma() const;

public Q_SLOTS:
    void setHandlePosition(int handleIndex, qreal newPosition) override;
    /**
     * @brief Sets the gamma value
     */
    void setGamma(qreal newGamma);
    /**
     * @see KisInputLevelsSlider::reset
     */
    void reset(qreal newBlackPoint, qreal newWhitePoint) override;
    /**
     * @see KisInputLevelsSlider::reset
     */
    void reset(qreal newBlackPoint, qreal newWhitePoint, qreal newGamma);

Q_SIGNALS:
    /**
     * @brief Signal emited when the gamma value changes
     */
    void gammaChanged(qreal newGamma);

protected:
    void paintBottomGradientMiddleSection(QImage &gradientImage, const QVector<Handle> &sortedHandles_) override;

private:
    qreal m_gamma;

    qreal gammaToPosition() const;
    qreal positionToGamma() const;
};

/**
 * @brief This is a simple output levels slider. It is basically the same as
 * KisInputLevelsSlider but the handles are not constrained and can move freely
 */
class KRITAWIDGETS_EXPORT KisOutputLevelsSlider : public KisInputLevelsSlider
{
    Q_OBJECT

public:
    KisOutputLevelsSlider(QWidget *parent = nullptr);
    ~KisOutputLevelsSlider();
};

/**
 * @brief This is a threshold slider that only has one handle
 */
class KRITAWIDGETS_EXPORT KisThresholdSlider : public KisInputLevelsSlider
{
    Q_OBJECT

public:
    KisThresholdSlider(QWidget *parent = nullptr);
    ~KisThresholdSlider();

    /**
     * @brief Get the gamma value
     */
    qreal threshold() const;

public Q_SLOTS:
    /**
     * @brief Sets the black point. For this slider the black and the white
     * points are always in the same position, so changing the black point will
     * also change the white point and viceversa
     */
    void setHandlePosition(int handleIndex, qreal newPosition) override;
    /**
     * @brief Sets the black point. For this slider the black and the white
     * points are always in the same position, so changing the black point will
     * also change the white point and viceversa
     */
    void setBlackPoint(qreal newBlackPoint) override;
    /**
     * @brief Sets the white point. For this slider the black and the white
     * points are always in the same position, so changing the black point will
     * also change the white point and viceversa
     */
    void setWhitePoint(qreal newWhitePoint) override;
    void reset(qreal newBlackPoint, qreal newWhitePoint) override;
    /**
     * @brief Sets the gamma value
     */
    void setThreshold(qreal newGamma);

Q_SIGNALS:
    /**
     * @brief Signal emited when the threshold value changes
     */
    void thresholdChanged(qreal newThreshold);

protected:
    void paintBottomGradientMiddleSection(QImage &gradientImage, const QVector<Handle> &sortedHandles_) override;
    void paintHandle(QPainter &painter, const QRect &rect, const Handle &handle) override;
};

#endif
