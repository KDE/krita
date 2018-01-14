/* This file is part of the KDE project
   Copyright (C) 2002, Rob Buis(buis@kde.org)
   Copyright (C) 2004, Nicolas GOUTTE <goutte@kde.org>
   Copyright (C) 2007, Thomas Zander <zander@kde.org>

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

#ifndef KOUNITDOUBLESPINBOX_H
#define KOUNITDOUBLESPINBOX_H

#include "kritawidgets_export.h"

#include <QDoubleSpinBox>

class KoUnit;

/**
 * Spin box for double precision numbers with unit display.
 * Use this widget for any value that represents a real measurable value for consistency throughout
 * Krita.
 * This widget shows the value in the user-selected units (inch, milimeters, etc) but keeps the
 * Krita-widget default measurement unit internally. This has the advantage that just setting and
 * getting a value will not change the value due to conversions.
 * The KoDocument class has a unit() method for consistent (document wide) configuration of the
 * used unit.
 * It is advised to use a QDoubleSpinBox in QtDesigner and then use the context-menu item: 'Promote to Custom Widget' and use the values: 'classname=KoUnitDoubleSpinBox', 'headerfile=KoUnitDoubleSpinBox.h'
 * This will generate code that uses this spinbox in the correct manner.
 *
 * This class need to be replaced as much as possible with \see KisDoubleParseUnitSpinBox to add math parsing ability.
 */
class KRITAWIDGETS_EXPORT KoUnitDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    /**
     * Constructor
     * Create a new spinBox with very broad range predefined.
     * This spinbox will have min and max borders of 10000 points and use
     * the default unit of points.
     * @param parent the parent widget
     */
    explicit KoUnitDoubleSpinBox( QWidget *parent = 0);
    ~KoUnitDoubleSpinBox() override;

    /**
     * Set the new value in points which will then be converted to the current unit for display
     * @param newValue the new value
     * @see value()
     */
    virtual void changeValue( double newValue );
    /**
     * This spinbox shows the internal value after a conversion to the unit set here.
     */
    virtual void setUnit( const KoUnit &);

    /// @return the current value, converted in points
    double value( ) const;

    /// Set minimum value in points.
    void setMinimum(double min);

    /// Set maximum value in points.
    void setMaximum(double max);

    /// Set step size in the current unit.
    void setLineStep(double step);

    /// Set step size in points.
    void setLineStepPt(double step);

    /// Set minimum, maximum value and the step size (all in points)
    void setMinMaxStep( double min, double max, double step );

    /// reimplemented from superclass, will forward to KoUnitDoubleValidator
    QValidator::State validate(QString &input, int &pos) const override;

    /**
     * Transform the double in a nice text, using locale symbols
     * @param value the number as double
     * @return the resulting string
     */
    QString textFromValue( double value ) const override;
    /**
     * Transform a string into a double, while taking care of locale specific symbols.
     * @param str the string to transform into a number
     * @return the value as double
     */
    double valueFromText( const QString& str ) const override;


Q_SIGNALS:
    /// emitted like valueChanged in the parent, but this one emits the point value
    void valueChangedPt( qreal );

private:
    class Private;
    Private * const d;

private Q_SLOTS:
    // exists to do emits for valueChangedPt
    void privateValueChanged();
};

#endif

