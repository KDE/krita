/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DOUBLEPARSEUNITSPINBOX_H
#define KIS_DOUBLEPARSEUNITSPINBOX_H

#include <KoUnit.h>

#include "kis_double_parse_spin_box.h"
#include "kritawidgets_export.h"

/*!
 * \brief The KisDoubleParseUnitSpinBox class is an evolution of the \see KoUnitDoubleSpinBox, but inherit from \see KisDoubleParseSpinBox to be able to parse math expressions.
 *
 * This class store the
 */
class KRITAWIDGETS_EXPORT  KisDoubleParseUnitSpinBox : public KisDoubleParseSpinBox
{

    Q_OBJECT

public:
    KisDoubleParseUnitSpinBox(QWidget* parent = 0);
    ~KisDoubleParseUnitSpinBox() override;

    /**
     * Set the new value in points which will then be converted to the current unit for display
     * @param newValue the new value
     * @see value()
     */
    virtual void changeValue( double newValue );
    /**
     * This spinbox shows the internal value after a conversion to the unit set here.
     */
    virtual void setUnit(const KoUnit &unit);

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
     * Transfrom a string into a double, while taking care of locale specific symbols.
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

#endif // KIS_DOUBLEPARSEUNITSPINBOX_H
