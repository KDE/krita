/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KIS_INT_SPINBOX_H_
#define KIS_INT_SPINBOX_H_

#include <qwidget.h>
#include <qspinbox.h>

#include <knumvalidator.h>

class QLabel;
class QSlider;
class QLineEdit;
class QLayout;
class QValidator;

/**
 * @short An input widget for integer numbers, consisting of a spinbox and
 * a dropdown slider.
 *
 * KisIntSpinbox combines a QSpinBox and a dropdown QSlider
 * to make an easy to use control for setting some integer
 * parameter.
 *
 * It uses KIntValidator validator class. KisIntSpinbox enforces the
 * value to be in the given range, and can display it in any base
 * between 2 and 36.
 *
 * This class is (apart from one constructor) API compatible with KIntNumInput
 * and I'll probably propose it as a replacement.
 */

class KisIntSpinbox : public QWidget
{

    Q_OBJECT
    Q_PROPERTY( int value READ value WRITE setValue )
    Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
    Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( int referencePoint READ referencePoint WRITE setReferencePoint )
    Q_PROPERTY( double relativeValue READ relativeValue WRITE setRelativeValue )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )

    /**
     * Constructs an input control for integer values
     * with base 10 and initial value 0.
     * 
     * @param label the tabel (may contain &, and my be empty)
     * @param parent parent QWidget
     * @param name   internal name for this widget
     */
    KisIntSpinbox(const QString & label, QWidget *parent=0, const char *name=0);
    /**
     * Constructor
     * It constructs a QSpinBox that allows the input of integer numbers
     * in the range of -INT_MAX to +INT_MAX. 
     * To enforce the value being in a range, use setRange().
     *
     * @param label the tabel (may contain &, and my be empty)
     * @param value  initial value for the control
     * @param base   numeric base used for display
     * @param parent parent QWidget
     * @param name   internal name for this widget
     */
    KisIntSpinbox(const QString & label, int value, int base = 10, QWidget* parent=0, const char *name=0);

    /**
     * Destructor
     *
     *
     */
    virtual ~KisIntSpinbox();
    
   /**
     * @return the current value.
     */
    int value() const;

    /**
     * @return the curent value in units of the referencePoint.
     * @since 3.1
     */
    double relativeValue() const;

    /**
     * @return the current reference point
     * @since 3.1
     */
    int referencePoint() const;

    /**
     * @return the suffix displayed behind the value.
     * @see setSuffix()
     */
    QString suffix() const;
    /**
     * @return the prefix displayed in front of the value.
     * @see setPrefix()
     */
    QString prefix() const;
    /**
     * @return the string displayed for a special value.
     * @see setSpecialValueText()
     */
    QString specialValueText() const;

    /**
     * @param min  minimum value
     * @param max  maximum value
     * @param step step size for the QSlider
     * @param slider whether the slider is created or not
     */
    void setRange(int min, int max, int step=1, bool slider=true);
    /**
     * Sets the minimum value.
     */
    void setMinValue(int min);
    /**
     * @return the minimum value.
     */
    int minValue() const;
    /**
     * Sets the maximum value.
     */
    void setMaxValue(int max);
    /**
     * @return the maximum value.
     */
    int maxValue() const;

    /**
     * Sets the special value text. If set, the SpinBox will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

    /**
     * Sets the spacing of tickmarks for the slider.
     *
     * @param minor Minor tickmark separation.
     * @param major Major tickmark separation.
     */
    void setSteps(int minor, int major);

    /**
     * This method returns the minimum size necessary to display the
     * control.
     *
     * @return the minimum size necessary to show the control
     */
    virtual QSize minimumSizeHint() const;

    void setLabel(const QString & label);

public slots:
    /**
     * Sets the value of the control.
     */
    void setValue(int);

    /**
     * Sets the value in units of the referencePoint
     * @since 3.1
     */
    void setRelativeValue(double);

    /**
     * Sets the reference point for relativeValue.
     * @since 3.1
     */
    void setReferencePoint(int);

    /**
     * Sets the suffix to @p suffix.
     * Use QString::null to disable this feature.
     * Formatting has to be provided (e.g. a space separator between the
     * prepended @p value and the suffix's text has to be provided
     * as the first character in the suffix).
     *
     * @see QSpinBox::setSuffix(), #setPrefix()
     */
    void setSuffix(const QString &suffix);

    /**
     * Sets the prefix to @p prefix.
     * Use QString::null to disable this feature.
     * Formatting has to be provided (see above).
     *
     * @see QSpinBox::setPrefix(), #setSuffix()
     */
    void setPrefix(const QString &prefix);

    /**
     * sets focus to the edit widget and marks all text in if mark == true
     *
     */
    void setEditFocus( bool mark = true );

signals:
    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     */
    void valueChanged(int);

    /**
     * Emitted whenever valueChanged is. Contains the change
     * relative to the referencePoint.
     * @since 3.1
     */
    void relativeValueChanged(double);

private:

    class KisIntSpinboxPrivate;
    KisIntSpinboxPrivate *d;                             

};

#endif
