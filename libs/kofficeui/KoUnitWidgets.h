/* This file is part of the KDE project
   Copyright (C) 2002, Rob Buis(buis@kde.org)
   Copyright (C) 2004, Nicolas GOUTTE <goutte@kde.org>

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

#ifndef __KOUNITWIDGETS_H__
#define __KOUNITWIDGETS_H__

#include <knuminput.h>
#include <knumvalidator.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <KoUnit.h>
#include <koffice_export.h>
//Added by qt3to4:
#include <QEvent>


// ----------------------------------------------------------------
//                          Support classes


class KoUnitDoubleBase;

// ### TODO: put it out of the public header file (if possible)
/**
 * Validator for the unit widget classes
 * \internal
 * \since 1.4 (change of behavior)
 */
class KOFFICEUI_EXPORT KoUnitDoubleValidator : public KDoubleValidator
{
public:
    KoUnitDoubleValidator( KoUnitDoubleBase *base, QObject *parent, const char *name = 0 );

    virtual QValidator::State validate( QString &, int & ) const;

private:
    KoUnitDoubleBase *m_base;
};


/**
 * Base for the unit widgets
 * \since 1.4 (change of behavior)
 */
class KOFFICEUI_EXPORT KoUnitDoubleBase
{
public:
    KoUnitDoubleBase( KoUnit::Unit unit, unsigned int precision ) : m_unit( unit ), m_precision( precision ) {}
    virtual ~KoUnitDoubleBase() {}

    virtual void changeValue( double ) = 0;
    virtual void setUnit( KoUnit::Unit = KoUnit::U_PT ) = 0;

    void setValueInUnit( double value, KoUnit::Unit unit )
    {
        changeValue( KoUnit::ptToUnit( KoUnit::fromUserValue( value, unit ), m_unit ) );
    }

    void setPrecision( unsigned int precision ) { m_precision = precision; };

protected:
    friend class KoUnitDoubleValidator;
    /**
     * Transform the double in a nice text, using locale symbols
     * @param value the number as double
     * @return the resulting string
     */
    QString getVisibleText( double value ) const;
    /**
     * Transfrom a string into a double, while taking care of locale specific symbols.
     * @param str the string to transform into a number
     * @param ok true, if the conversion was succesful
     * @return the value as double
     */
    double toDouble( const QString& str, bool* ok ) const;

protected:
    KoUnitDoubleValidator *m_validator;
    KoUnit::Unit m_unit;
    unsigned int m_precision;
};


// ----------------------------------------------------------------
//                          Widget classes


/**
 * Spin box for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KOFFICEUI_EXPORT KoUnitDoubleSpinBox : public KDoubleSpinBox, public KoUnitDoubleBase
{
    Q_OBJECT
public:
    KoUnitDoubleSpinBox( QWidget *parent = 0L, const char *name = 0L );
    // lower, upper, step and value are in pt
    KoUnitDoubleSpinBox( QWidget *parent, double lower, double upper, double step, double value = 0.0,
                         KoUnit::Unit unit = KoUnit::U_PT, unsigned int precision = 2, const char *name = 0 );
    // added so the class can be used in .ui files(by Tymoteusz Majewski, maju7@o2.pl)
    virtual void changeValue( double );
    virtual void setUnit( KoUnit::Unit = KoUnit::U_PT );

    /// @return the current value, converted in points
    double value( void ) const;

    /// Set minimum value in points.
    void setMinValue(double min);

    /// Set maximum value in points.
    void setMaxValue(double max);

    /// Set step size in the current unit.
    void setLineStep(double step);

    /// Set step size in points.
    void setLineStepPt(double step);

    /// Set minimum, maximum value and the step size (all in points) (by Tymoteusz Majewski, maju7@o2.pl)
    void setMinMaxStep( double min, double max, double step );

signals:
    /// emitted like valueChanged in the parent, but this one emits the point value
    void valueChangedPt( double );


private:
    double m_lowerInPoints; ///< lowest value in points
    double m_upperInPoints; ///< highest value in points
    double m_stepInPoints;  ///< step in points

private slots:
    // exists to do emits for valueChangedPt
    void privateValueChanged();
};


/**
 * Line edit for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KOFFICEUI_EXPORT KoUnitDoubleLineEdit : public KLineEdit, public KoUnitDoubleBase
{
    Q_OBJECT
public:
    KoUnitDoubleLineEdit( QWidget *parent = 0L, const char *name = 0L );
    KoUnitDoubleLineEdit( QWidget *parent, double lower, double upper, double value = 0.0, KoUnit::Unit unit = KoUnit::U_PT, unsigned int precision = 2, const char *name = 0 );

    virtual void changeValue( double );
    virtual void setUnit( KoUnit::Unit = KoUnit::U_PT );

    /// @return the current value, converted in points
    double value( void ) const;

protected:
    bool eventFilter( QObject* obj, QEvent* ev );

private:
    double m_value;
    double m_lower;
    double m_upper;
    double m_lowerInPoints; ///< lowest value in points
    double m_upperInPoints; ///< highest value in points
};

/**
 * Combo box for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KOFFICEUI_EXPORT KoUnitDoubleComboBox : public KComboBox, public KoUnitDoubleBase
{
    Q_OBJECT
public:
    KoUnitDoubleComboBox( QWidget *parent = 0L, const char *name = 0L );
    KoUnitDoubleComboBox( QWidget *parent, double lower, double upper, double value = 0.0, KoUnit::Unit unit = KoUnit::U_PT, unsigned int precision = 2, const char *name = 0 );

    virtual void changeValue( double );
    void updateValue( double );
    virtual void setUnit( KoUnit::Unit = KoUnit::U_PT );

    /// @return the current value, converted in points
    double value( void ) const;
    void insertItem( double, int index = -1 );

protected:
    bool eventFilter( QObject* obj, QEvent* ev );

signals:
    void valueChanged(double);

private slots:
    void slotActivated( int );

protected:
    double m_value;
    double m_lower;
    double m_upper;
    double m_lowerInPoints; ///< lowest value in points
    double m_upperInPoints; ///< highest value in points
};

/**
 * Combo box (with spin control) for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KOFFICEUI_EXPORT KoUnitDoubleSpinComboBox : public QWidget
{
    Q_OBJECT
public:
    KoUnitDoubleSpinComboBox( QWidget *parent = 0L, const char *name = 0L );
    KoUnitDoubleSpinComboBox( QWidget *parent, double lower, double upper, double step, double value = 0.0, KoUnit::Unit unit = KoUnit::U_PT, unsigned int precision = 2, const char *name = 0 );

    void insertItem( double, int index = -1 );
    void updateValue( double );
    /// @return the current value, converted in points
    double value( void ) const;

signals:
    void valueChanged(double);

private slots:
    void slotUpClicked();
    void slotDownClicked();

private:
    KoUnitDoubleComboBox *m_combo;
    double m_step;
};

#endif

