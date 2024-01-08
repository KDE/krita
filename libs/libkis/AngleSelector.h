/*
 *  SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef LIBKIS_ANGLESELECTOR_H
#define LIBKIS_ANGLESELECTOR_H

#include "KisAngleSelector.h"

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * @brief A wrapper around KisAngleSelector, a widget with several options to
 * select an angle. The widget itself is accessed with the widget() function.
 * 
 * This widget is a combination of a @ref KisAngleGauge and a spin box,
 * along with some flipping options
 */
class KRITALIBKIS_EXPORT AngleSelector : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AngleSelector)

public:
    explicit AngleSelector();
    ~AngleSelector() override;

public Q_SLOTS:

    /**
     * @brief Get the internal KisAngleSelector as a QWidget, so it may be
     * added to a UI
     * 
     * @return the internal KisAngleSelector as a QWidget
     */
    QWidget* widget() const;

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
     * @brief Gets the number of decimals (precision) used by the angle
     * 
     * If you want to simulate integer angles, set it to 0. The default is 2.
     * @return The number of decimals being used
     * @see setDecimals(int)
     */
    int	decimals() const;
    /**
     * @brief Gets the maximum value for the angle
     * 
     * The default is 360
     * @return The maximum value for the angle
     * @see setMaximum(qreal)
     * @see setRange(qreal, qreal)
     */
    qreal maximum() const;
    /**
     * @brief Gets the minimum value for the angle
     * 
     * The default is 0
     * @return The minimum value for the angle
     * @see setMinimum(qreal)
     * @see setRange(qreal, qreal)
     */
    qreal minimum() const;
    /**
     * @brief Gets the prefix shown in the spin box
     * @return The prefix shown in the spin box
     * @see setPrefix(const QString&)
     */
    QString prefix() const;
    /**
     * @brief Gets if the angle should wrap pass the minimum or maximum angles
     * @return True if the angle should wrap pass the minimum or maximum angles,
     * false otherwise
     * @see setWrapping(bool)
     */
    bool wrapping() const;
    /**
     * @brief Gets the mode in which the flip options should be shown
     * 
     * The default is Buttons
     * @return The mode in which the flip options should be shown.
     * @see setFlipOptionsMode(QString)
     */
    QString flipOptionsMode() const;
    /**
     * @brief Gets the common height of the widgets inside this angle selector
     * @return The height of the internal widgets (angle gauge, spin box, etc.).
     *         Returns 0 if each widget has its default height.
     * @see setWidgetsHeight(int)
     */
    int widgetsHeight() const;
    /**
     * @brief Gets the direction in which the angle increases in the angle gauge
     * @return The direction in which the angle increases
     * @see setIncreasingDirection(QString)
     */
    QString increasingDirection() const;
    /**
     * @brief Gets if the spin box is flat (no border or background)
     * @return True if the spin box is flat, false otherwise
     * @see useFlatSpinBox(bool)
     */
    bool isUsingFlatSpinBox() const;
    
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
     * @brief Sets the number of decimals (precision) used by the angle
     * @param newNumberOfDecimals the new number of decimals used by the angle
     * @see decimals() const
     */
    void setDecimals(int newNumberOfDecimals);
    /**
     * @brief Sets the maximum value for the angle
     * @param newMaximum the new maximum value for the angle
     * @see maximum() const
     * @see setRange(qreal, qreal)
     */
    void setMaximum(qreal newMaximum);
    /**
     * @brief Sets the minimum value for the angle
     * @param newMinimum the new minimum value for the angle
     * @see minimum() const
     * @see setRange(qreal, qreal)
     */
    void setMinimum(qreal newMinimum);
    /**
     * @brief Sets the minimum and maximum values for the angle
     * @param newMinimum the new minimum value for the angle
     * @param newMaximum the new maximum value for the angle
     * @see minimum() const
     * @see maximum() const
     * @see setMinimum(qreal)
     * @see setMaximum(qreal)
     */
    void setRange(qreal newMinimum, qreal newMaximum);
    /**
     * @brief Sets the prefix shown in the spin box
     * @param newPrefix the new prefix for the spin box
     * @see prefix() const
     */
    void setPrefix(const QString &newPrefix);
    /**
     * @brief Sets if the angle should wrap pass the minimum or maximum angles
     * @param newWrapping true if the angle should wrap pass the minimum or
     * maximum angles, false otherwise
     * @see wrapping() const
     */
    void setWrapping(bool newWrapping);
    /**
     * @brief Sets the mode in which the flip options should be shown
     * @param newMode the new mode in which the flip options should be shown
     * Valid arguments:
     * <ul>
     * <li>NoFlipOptions - There are no flip options available</li>
     * <li>MenuButton - The flip options are shown as a menu accessible via a options button</li>
     * <li>Buttons - The flip options are shown as individual buttons</li>
     * <li>ContextMenu - The flip options are shown only as a context menu when right-clicking the gauge widget</li>
     * <ul>
     * 
     * @see flipOptionsMode() const
     */
    void setFlipOptionsMode(QString newMode);
    /**
     * @brief Sets the common height of the widgets inside this angle selector.
     *        Use 0 to reset widgets to default height.
     * @param newHeight the new height of the internal widgets (angle gauge, spin box, etc.)
     * @see widgetsHeight() const
     */
    void setWidgetsHeight(int newHeight);
    /**
     * @brief Sets the increasing direction in the angle gauge
     * @param newIncreasingDirection The new increasing direction
     * Valid arguments: CounterClockwise or Clockwise.
     * @see increasingDirection() const
     */
    void setIncreasingDirection(QString newIncreasingDirection);
    /**
     * @brief Sets if the spin box should be flat
     * @param newUseFlatSpinBox True if the spin box should be flat,
     * false otherwise
     * @see isUsingFlatSpinBox() const
     */
    void useFlatSpinBox(bool newUseFlatSpinBox);

    /**
     * @brief Gets the closest coterminal angle to the provided angle
     * that is in the range provided
     * 
     * A coterminal angle to the provided angle is one that differs
     * in size by an integer multiple of a turn (360 degrees)
     * @param angle The reference angle for which the function will try to
     * find a coterminal angle
     * @param minimum The range's lower bound
     * @param maximum The range's upper bound
     * @param[out] ok This parameter will be set to true if a coterminal
     * angle exists in the provided range, or to false otherwise
     * @return The closest coterminal angle in the provided range if one exists,
     * or the closest value in the range (the minimum or maximum) otherwise.
     * If the reference angle is already in the range then it is returned
     */
    static qreal closestCoterminalAngleInRange(qreal angle, qreal minimum, qreal maximum, bool *ok = nullptr);
    /**
     * @brief Gets the closest coterminal angle to the provided angle
     * that is in the range established
     * 
     * A coterminal angle to the provided angle is one that differs
     * in size by an integer multiple of a turn (360 degrees)
     * @param angle The reference angle for which the function will try to
     * find a coterminal angle
     * @param[out] ok This parameter will be set to true if a coterminal
     * angle exists in the specified range, or to false otherwise
     * @return The closest coterminal angle in the specified range if one exists,
     * or the closest value in the range (the minimum or maximum) otherwise.
     * If the reference angle is already in the range then it is returned
     */
    qreal closestCoterminalAngleInRange(qreal angle, bool *ok = nullptr) const;
    /**
     * @brief Flips an angle horizontally, vertically, or both
     * 
     * This function will always try to get the closest angle to the
     * provided one that satisfies the flipping requirements
     * @param angle The angle to be flipped
     * @param orientations Flags indicating in which directions the angle should
     * be flipped
     * @return The flipped angle
     */
    static qreal flipAngle(qreal angle, Qt::Orientations orientations);
    /**
     * @brief Flips an angle horizontally, vertically, or both
     * 
     * This function will always try to get the closest angle to the
     * provided one that satisfies the flipping requirements
     * @param angle The angle to be flipped
     * @param minimum The lower bound of the valid range
     * @param maximum The upper bound of the valid range
     * @param orientations Flags indicating in which directions the angle should
     * be flipped
     * @param[out] ok This parameter will be set to true if the flipped
     * angle is in the provided range, or to false otherwise
     * @return The flipped angle if it lies in the provided range or the
     * closest value in the range (the minimum or maximum) otherwise
     */
    static qreal flipAngle(qreal angle, qreal minimum, qreal maximum, Qt::Orientations orientations, bool *ok = nullptr);
    /**
     * @brief Flips the angle horizontally, vertically, or both
     * 
     * This function will always try to set the closest angle to the
     * stablished one that satisfies the flipping requirements
     * @param orientations Flags indicating in which directions the angle should
     * be flipped
     */
    void flip(Qt::Orientations orientations);

Q_SIGNALS:
    void angleChanged(qreal angle);

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_ANGLESELECTOR_H
