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

#include "KoUnitDoubleSpinBox.h"
#include <kdebug.h>
#include <KGlobal>
#include <KLocale>

// #define DEBUG_VALIDATOR

class KoUnitDoubleSpinBox::Private {
public:
    Private(double low, double up, double step)
        : lowerInPoints(low),
        upperInPoints(up),
        stepInPoints(step),
        unit(KoUnit(KoUnit::Point))
    {
    }

    double lowerInPoints; ///< lowest value in points
    double upperInPoints; ///< highest value in points
    double stepInPoints;  ///< step in points
    KoUnit unit;
};

KoUnitDoubleSpinBox::KoUnitDoubleSpinBox( QWidget *parent)
    : QDoubleSpinBox( parent ),
    d( new Private(-9999, 9999, 1))
{
    QDoubleSpinBox::setDecimals( 2 );
    //setAcceptLocalizedNumbers( true );
    setUnit( KoUnit(KoUnit::Point) );
    setAlignment( Qt::AlignRight );

    connect(this, SIGNAL(valueChanged( double )), SLOT(privateValueChanged()));
}

KoUnitDoubleSpinBox::~KoUnitDoubleSpinBox()
{
    delete d;
}

// deprecated;
KoUnitDoubleSpinBox::KoUnitDoubleSpinBox( QWidget *parent,
						    double lower, double upper,
						    double step,
						    double value,
						    KoUnit unit,
                            unsigned int precision)
    : QDoubleSpinBox( parent ),
    d( new Private(lower, upper, step))
{
    setMinimum(lower);
    setMaximum(upper);
    setSingleStep(step);
    setValue(value);
    setDecimals(precision);
    d->unit = KoUnit(KoUnit::Point);
    //setAcceptLocalizedNumbers( true );
    setUnit( unit );
    changeValue( value );
    setLineStepPt( step );

    connect(this, SIGNAL(valueChanged( double )), SLOT(privateValueChanged()));
}

QValidator::State KoUnitDoubleSpinBox::validate(QString &input, int &pos) const
{
#ifdef DEBUG_VALIDATOR
    kDebug(30004) <<"KoUnitDoubleSpinBox::validate :" << input <<" at" << pos;
#else
    Q_UNUSED(pos);
#endif

    QRegExp regexp ("([ a-zA-Z]+)$"); // Letters or spaces at end
    const int res = input.indexOf( regexp );

    if ( res == -1 )
    {
        // Nothing like an unit? The user is probably editing the unit
#ifdef DEBUG_VALIDATOR
        kDebug(30004) <<"Intermediate (no unit)";
#endif
        return QValidator::Intermediate;
    }

    // ### TODO: are all the QString::trimmed really necessary?
    const QString number ( input.left( res ).trimmed() );
    const QString unitName ( regexp.cap( 1 ).trimmed().toLower() );

#ifdef DEBUG_VALIDATOR
    kDebug(30004) <<"Split:" << number <<":" << unitName <<":";
#endif

    const double value = valueFromText( number );
    double newVal = 0.0;
    if( value != NAN )
    {
        bool ok;
        KoUnit unit = KoUnit::unit( unitName, &ok );
        if ( ok )
            newVal = unit.fromUserValue( value );
        else
        {
            // Probably the user is trying to edit the unit
#ifdef DEBUG_VALIDATOR
            kDebug(30004) <<"Intermediate (unknown unit)";
#endif
            return QValidator::Intermediate;
        }
    }
    else
    {
        kWarning(30004) << "Not a number: " << number;
        return QValidator::Invalid;
    }
    newVal = KoUnit::ptToUnit( newVal, d->unit );
    //input = textFromValue( newVal ); // don't overwrite for now; the effect is not exactly what I expect...

    return QValidator::Acceptable;
}

void KoUnitDoubleSpinBox::changeValue( double val )
{
    QDoubleSpinBox::setValue( d->unit.toUserValue( val ) );
    // TODO: emit valueChanged ONLY if the value was out-of-bounds
    // This will allow the 'user' dialog to set a dirty bool and ensure
    // a proper value is getting saved.
}

void KoUnitDoubleSpinBox::privateValueChanged() {
    emit valueChangedPt( value () );
}

void KoUnitDoubleSpinBox::setUnit( KoUnit unit )
{
    double oldvalue = d->unit.fromUserValue( QDoubleSpinBox::value() );
    QDoubleSpinBox::setMinimum( unit.toUserValue( d->lowerInPoints ) );
    QDoubleSpinBox::setMaximum( unit.toUserValue( d->upperInPoints ) );
    QDoubleSpinBox::setSingleStep( unit.toUserValue( d->stepInPoints ) );
    d->unit = unit;
    QDoubleSpinBox::setValue( KoUnit::ptToUnit( oldvalue, unit ) );
    setSuffix( KoUnit::unitName( unit ).prepend( ' ' ) );
}

double KoUnitDoubleSpinBox::value( ) const
{
    return d->unit.fromUserValue( QDoubleSpinBox::value() );
}

void KoUnitDoubleSpinBox::setMinimum( double min )
{
  d->lowerInPoints = min;
  QDoubleSpinBox::setMinimum( d->unit.toUserValue( min ) );
}

void KoUnitDoubleSpinBox::setMaximum( double max )
{
  d->upperInPoints = max;
  QDoubleSpinBox::setMaximum( d->unit.toUserValue( max ) );
}

void KoUnitDoubleSpinBox::setLineStep( double step )
{
  d->stepInPoints = KoUnit(KoUnit::Point).toUserValue(step);
  QDoubleSpinBox::setSingleStep( step );
}

void KoUnitDoubleSpinBox::setLineStepPt( double step )
{
  d->stepInPoints = step;
  QDoubleSpinBox::setSingleStep( d->unit.toUserValue( step ) );
}

void KoUnitDoubleSpinBox::setMinMaxStep( double min, double max, double step )
{
  setMinimum( min );
  setMaximum( max );
  setLineStepPt( step );
}

QString KoUnitDoubleSpinBox::textFromValue( double value ) const
{
    //kDebug(30004) <<"textFromValue:" << QString::number( value, 'f', 12 ) <<" =>" << num;
    //const QString num ( QString( "%1%2").arg( KGlobal::locale()->formatNumber( value, d->precision ), KoUnit::unitName( m_unit ) ) );
    //const QString num ( QString( "%1").arg( KGlobal::locale()->formatNumber( value, d->precision )) );
    return KGlobal::locale()->formatNumber( value, decimals() );
}

double KoUnitDoubleSpinBox::valueFromText( const QString& str ) const
{
    QString str2( str );
    /* KLocale::readNumber wants the thousand separator exactly at 1000.
       But when editing, it might be anywhere. So we need to remove it. */
    const QString sep( KGlobal::locale()->thousandsSeparator() );
    if ( !sep.isEmpty() )
        str2.remove( sep );
    str2.remove( KoUnit::unitName( d->unit ) );
    bool ok;
    const double dbl = KGlobal::locale()->readNumber( str2, &ok );
    if ( ok )
      kDebug(30004) <<"valueFromText:" << str <<": => :" << str2 <<": =>" << QString::number( dbl, 'f', 12 );
    else
        kWarning(30004) << "valueFromText error:" << str << ": => :" << str2 << ":";
    return dbl;
}



#include <KoUnitDoubleSpinBox.moc>
