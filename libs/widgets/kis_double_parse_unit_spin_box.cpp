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

#include "kis_double_parse_unit_spin_box.h"


class Q_DECL_HIDDEN KisDoubleParseUnitSpinBox::Private
{
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

KisDoubleParseUnitSpinBox::KisDoubleParseUnitSpinBox(QWidget *parent) :
    KisDoubleParseSpinBox(parent),
    d(new Private(-9999, 9999, 1))
{
    setUnit( KoUnit(KoUnit::Point) );
    setAlignment( Qt::AlignRight );

    connect(this, SIGNAL(valueChanged( double )), SLOT(privateValueChanged()));
}

KisDoubleParseUnitSpinBox::~KisDoubleParseUnitSpinBox()
{
    delete d;
}

void KisDoubleParseUnitSpinBox::changeValue( double newValue )
{
    if (d->unit.toUserValue(newValue) == oldValue) {
        return;
    }

    KisDoubleParseSpinBox::setValue( d->unit.toUserValue(newValue) );
}

void KisDoubleParseUnitSpinBox::setUnit( const KoUnit & unit)
{
    if( unit == d->unit) return;
    double oldValue = d->unit.fromUserValue( KisDoubleParseSpinBox::value() );

    KisDoubleParseSpinBox::setMinimum( unit.toUserValue( d->lowerInPoints ) );
    KisDoubleParseSpinBox::setMaximum( unit.toUserValue( d->upperInPoints ) );

    qreal step = unit.toUserValue( d->stepInPoints );

    if (unit.type() == KoUnit::Pixel) {
        // limit the pixel step by 1.0
        step = qMax(qreal(1.0), step);
    }

    KisDoubleParseSpinBox::setSingleStep( step );
    d->unit = unit;
    KisDoubleParseSpinBox::setValue( KoUnit::ptToUnit( oldValue, unit ) );
    setSuffix( unit.symbol().prepend(QLatin1Char(' ')) );
}

double KisDoubleParseUnitSpinBox::value( ) const
{
    return d->unit.fromUserValue( KisDoubleParseSpinBox::value() );
}

void KisDoubleParseUnitSpinBox::setMinimum(double min)
{
    d->lowerInPoints = min;
    KisDoubleParseSpinBox::setMinimum( d->unit.toUserValue( min ) );
}

void KisDoubleParseUnitSpinBox::setMaximum(double max)
{
    d->upperInPoints = max;
    KisDoubleParseSpinBox::setMaximum( d->unit.toUserValue( max ) );
}

void KisDoubleParseUnitSpinBox::setLineStep(double step)
{
    d->stepInPoints = KoUnit(KoUnit::Point).toUserValue(step);
    KisDoubleParseSpinBox::setSingleStep( step );
}

void KisDoubleParseUnitSpinBox::setLineStepPt(double step)
{
    d->stepInPoints = step;
    KisDoubleParseSpinBox::setSingleStep( d->unit.toUserValue( step ) );
}


void KisDoubleParseUnitSpinBox::setMinMaxStep( double min, double max, double step )
{
    setMinimum( min );
    setMaximum( max );
    setLineStepPt( step );
}


QValidator::State KisDoubleParseUnitSpinBox::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos);

    QRegExp regexp ("([ a-zA-Z]+)$"); // Letters or spaces at end
    const int res = input.indexOf( regexp );

    if ( res == -1 ) {
        // Nothing like an unit? The user is probably editing the unit
        return QValidator::Intermediate;
    }

    QString expr ( input.left( res ) );
    const QString unitName ( regexp.cap( 1 ).trimmed().toLower() );

    bool ok = true;
    bool interm = false;

    QValidator::State exprState = KisDoubleParseSpinBox::validate(expr, pos);

    if (exprState == QValidator::Invalid) {
        return exprState;
    } else if (exprState == QValidator::Intermediate) {
        interm = true;
    }

    //check if we can parse the unit.
    KoUnit::fromSymbol(unitName, &ok);

    if (!ok || interm) {
        return QValidator::Intermediate;
    }

    return QValidator::Acceptable;
}

QString KisDoubleParseUnitSpinBox::textFromValue( double value ) const
{
    return KisDoubleParseSpinBox::textFromValue(value);
}

double KisDoubleParseUnitSpinBox::valueFromText( const QString& str ) const
{
    //KisDoubleParseSpinBox is supposed to remove the suffix and prefix by itself.
    return KisDoubleParseSpinBox::valueFromText(str);
}

void KisDoubleParseUnitSpinBox::privateValueChanged()
{
    emit valueChangedPt( value() );
}
