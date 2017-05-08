/* This file is part of the KDE project
   Copyright (c) 2007 C. Boemann <cbo@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSLIDERCOMBO_H_
#define KOSLIDERCOMBO_H_

#include <QComboBox>

#include "kritawidgets_export.h"

/**
 * @short A widget for qreal values with a popup slider
 *
 * KoSliderCombo combines a numerical input and a dropdown slider in a way that takes up as
 * little screen space as possible.
 * 
 * It allows the user to either enter a floating point value or quickly set the value using a slider
 * 
 * One signal is emitted when the value changes. The signal is even emitted when the slider
 * is moving. The second argument of the signal however tells you if the value is final or not. A
 * final value is produced by entering a value numerically or by releasing the slider.
 * 
 * The input of the numerical line edit is constrained to numbers and decimal signs.
 */
class KRITAWIDGETS_EXPORT KoSliderCombo : public QComboBox
{

    Q_OBJECT

public:

    /**
     * Constructor for the widget, where value is set to 0
     *
     * @param parent parent QWidget
     */
    explicit KoSliderCombo(QWidget *parent=0);

    /**
     * Destructor
     */
    ~KoSliderCombo() override;

    /**
     * The precision of values given as the number of digits after the period.
     * default is 2
     */
    qreal decimals() const;

    /**
     * The minimum value that can be entered.
     * default is 0
     */
    qreal minimum() const;

    /**
     * The maximum value that can be entered.
     * default is 100
     */
    qreal maximum() const;

    /**
     * Sets the precision of the entered values.
     * @param number the number of digits after the period
     */

    void setDecimals(int number);

    /**
     * Sets the minimum value that can be entered.
     * @param min the minimum value
     */
    void setMinimum(qreal min);

    /**
     * Sets the maximum value that can be entered.
     * @param max the maximum value
     */
    void setMaximum(qreal max);

     /**
     * The value shown.
     */
    qreal value() const;

    QSize minimumSizeHint() const override; ///< reimplemented from QComboBox
    QSize sizeHint() const override; ///< reimplemented from QComboBox

public Q_SLOTS:

     /**
     * Sets the value.
     * The value actually set is forced to be within the legal range: minimum <= value <= maximum
     * @param value the new value
     */
    void setValue(qreal value);

Q_SIGNALS:

    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     * @param value the new value
     * @param final if the value is final ie not produced during sliding (on slider release it's final)
     */
    void valueChanged(qreal value, bool final);

protected:
    void paintEvent(QPaintEvent *) override; ///< reimplemented from QComboBox
    void hideEvent(QHideEvent *) override; ///< reimplemented from QComboBox
    void changeEvent(QEvent *e) override; ///< reimplemented from QComboBox
    void mousePressEvent(QMouseEvent *e) override; ///< reimplemented from QComboBox
    void keyPressEvent(QKeyEvent *e) override; ///< reimplemented from QComboBox
    void wheelEvent(QWheelEvent *e) override; ///< reimplemented from QComboBox

private:
    Q_PRIVATE_SLOT(d, void sliderValueChanged(int value))
    Q_PRIVATE_SLOT(d, void sliderReleased())
    Q_PRIVATE_SLOT(d, void lineEditFinished())

    class KoSliderComboPrivate;
    KoSliderComboPrivate * const d;
};

#endif
