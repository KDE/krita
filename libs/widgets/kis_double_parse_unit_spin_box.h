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

class KisSpinBoxUnitManager;

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

    void setUnitManager(KisSpinBoxUnitManager* unitManager);

    /**
     * Set the new value in points (or other reference unit) which will then be converted to the current unit for display
     * @param newValue the new value
     * @see value()
     */
    virtual void changeValue( double newValue );

    /**
     * This spinbox shows the internal value after a conversion to the unit set here.
     */
    virtual void setUnit(const KoUnit &unit);
    virtual void setUnit(const QString & symbol);
    /*!
     * \brief setReturnUnit set a unit, such that the spinbox now return values in this unit instead of the reference unit for the current dimension.
     * \param symbol the symbol of the new unit.
     */
    void setReturnUnit(const QString & symbol);

    /**
     * @brief setDimensionType set the dimension (for example length or angle) of the units the spinbox manage
     * @param dim the dimension id. (if not an id in KisSpinBoxUnitManager::UnitDimension, then the function does nothing).
     */
    virtual void setDimensionType(int dim);

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

    //! \brief get the text in the spinbox without prefix or suffix, and remove unit symbol if present.
    QString veryCleanText() const override;

    /**
     * Transform a string into a double, while taking care of locale specific symbols.
     * @param str the string to transform into a number
     * @return the value as double
     */
    double valueFromText( const QString& str ) const override;

    void setUnitChangeFromOutsideBehavior(bool toggle); //if set to false, setting the unit using KoUnit won't have any effect.

    //! \brief display the unit symbol in the spinbox or not. For example if the unit is displayed in a combobox connected to the unit manager.
    void setDisplayUnit(bool toggle);

    void preventDecimalsChangeFromUnitManager(bool prevent);

Q_SIGNALS:
    /// emitted like valueChanged in the parent, but this one emits the point value, or converted to another reference unit.
    void valueChangedPt( qreal );


private:
    class Private;
    Private * const d;

    QString detectUnit();
    QString makeTextClean(QString const& txt) const;

    //those functions are useful to sync the spinbox with its unitmanager.
    //! \brief change the unit, reset the spin box everytime. From the outside it's always set unit that should be called.
    void internalUnitChange(QString const& symbol);
    void prepareUnitChange();

private Q_SLOTS:
    // exists to do emits for valueChangedPt
    void privateValueChanged();
    void detectUnitChanges();
    void disconnectExternalUnitManager();

};

#endif // KIS_DOUBLEPARSEUNITSPINBOX_H
