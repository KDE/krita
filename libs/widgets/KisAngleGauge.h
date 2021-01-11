/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISANGLEGAUGE_H
#define KISANGLEGAUGE_H

#include <QWidget>
#include <QScopedPointer>

#include "kritawidgets_export.h"

/**
 * @brief A circular widget that allows to choose an angle
 */
class KRITAWIDGETS_EXPORT KisAngleGauge : public QWidget
{
    Q_OBJECT

public:
    enum IncreasingDirection
    {
        IncreasingDirection_CounterClockwise,
        IncreasingDirection_Clockwise
    };

    /**
     * @brief Construct a new KisAngleGauge widget
     * @param parent the parent widget
     */
    explicit KisAngleGauge(QWidget *parent = 0);
    ~KisAngleGauge();
    
    /**
     * @brief Gets the current angle
     * @return The current angle 
     * @see setAngle(qreal)
     */
    qreal angle() const;
    /**
     * @brief Gets the angle to which multiples the selected angle will snap
     * 
     * The default snap angle is 15 degrees so the selected angle will snap
     * to its multiples (0, 15, 30, 45, etc.)
     * @return The angle to which multiples the selected angle will snap
     * @see setSnapAngle(qreal)
     */
    qreal snapAngle() const;
    /**
     * @brief Gets the angle that is used to reset the current angle
     * 
     * This angle is used when the user double clicks on the widget
     * @return The angle that is used to reset the current angle
     * @see setResetAngle(qreal)
     */
    qreal resetAngle() const;
    /**
     * @brief Gets the direction in which the angle increases
     * @return The direction in which the angle increases
     * @see IcreasingDirection
     * @see setIncreasingDirection(IcreasingDirection)
     */
    IncreasingDirection increasingDirection() const;
    
    /**
     * @brief Sets the angle to which multiples the selected angle will snap
     * @param newSnapAngle the new angle to which multiples the selected angle will snap
     * @see snapAngle() const
     */
    void setSnapAngle(qreal newSnapAngle);
    /**
     * @brief Sets the angle that is used to reset the current angle
     * @param newResetAngle the new angle that is used to reset the current angle
     * @see resetAngle() const
     */
    void setResetAngle(qreal newResetAngle);
    /**
     * @brief Sets the increasing direction
     * @param newIncreasingDirection The new increasing direction
     * @see IcreasingDirection
     * @see increasingDirection() const
     */
    void setIncreasingDirection(IncreasingDirection newIncreasingDirection);

public Q_SLOTS:
    /**
     * @brief Sets the current angle
     * @param newAngle the new angle
     * @see angle() const
     */
    void setAngle(qreal newAngle);
    /**
     * @brief Sets the current angle to the reset angle
     * @see resetAngle() const
     * @see setResetAngle(qreal) const
     */
    void reset();

Q_SIGNALS:
    /**
     * @brief Signal emited when the angle has changed
     * @param angle The new angle
     */
    void angleChanged(qreal angle);

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
