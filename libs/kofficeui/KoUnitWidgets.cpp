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

#include "KoUnitWidgets.h"
#include "KoUnitWidgets.moc"
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QEvent>


// ----------------------------------------------------------------
//                          Support classes


KoUnitDoubleValidator::KoUnitDoubleValidator( KoUnitDoubleBase *base, QObject *parent, const char *name )
: KDoubleValidator( parent ), m_base( base )
{
}

QValidator::State
KoUnitDoubleValidator::validate( QString &s, int &pos ) const
{

    kDebug(30004) << "KoUnitDoubleValidator::validate : " << s << " at " << pos << endl;
    QValidator::State result = Acceptable;

    QRegExp regexp ("([ a-zA-Z]+)$"); // Letters or spaces at end
    const int res = regexp.search( s );

    if ( res == -1 )
    {
        // Nothing like an unit? The user is probably editing the unit
        kDebug(30004) << "Intermediate (no unit)" << endl;
        return Intermediate;
    }

    // ### TODO: are all the QString::trimmed really necessary?
    const QString number ( s.left( res ).trimmed() );
    const QString unitName ( regexp.cap( 1 ).trimmed().lower() );

    kDebug(30004) << "Split:" << number << ":" << unitName << ":" << endl;

    bool ok = false;
    const double value = m_base->toDouble( number, &ok );
    double newVal = 0.0;
    if( ok )
    {
        KoUnit::Unit unit = KoUnit::unit( unitName, &ok );
        if ( ok )
            newVal = KoUnit::fromUserValue( value, unit );
        else
        {
            // Probably the user is trying to edit the unit
            kDebug(30004) << "Intermediate (unknown unit)" << endl;
            return Intermediate;
        }
    }
    else
    {
        kWarning(30004) << "Not a number: " << number << endl;
        return Invalid;
    }

    newVal = KoUnit::ptToUnit( newVal, m_base->m_unit );

    s = m_base->getVisibleText( newVal );

    return result;
}


QString KoUnitDoubleBase::getVisibleText( double value ) const
{
    const QString num ( QString( "%1%2").arg( KGlobal::locale()->formatNumber( value, m_precision ), KoUnit::unitName( m_unit ) ) );
    kDebug(30004) << "getVisibleText: " << QString::number( value, 'f', 12 ) << " => " << num << endl;
    return num;
}

double KoUnitDoubleBase::toDouble( const QString& str, bool* ok ) const
{
    QString str2( str );
    /* KLocale::readNumber wants the thousand separator exactly at 1000.
       But when editing, it might be anywhere. So we need to remove it. */
    const QString sep( KGlobal::locale()->thousandsSeparator() );
    if ( !sep.isEmpty() )
        str2.remove( sep );
    str2.remove( KoUnit::unitName( m_unit ) );
    const double dbl = KGlobal::locale()->readNumber( str2, ok );
    if ( ok )
      kDebug(30004) << "toDouble:" << str << ": => :" << str2 << ": => " << QString::number( dbl, 'f', 12 ) << endl;
    else
        kWarning(30004) << "toDouble error:" << str << ": => :" << str2 << ":" << endl;
    return dbl;
}


// ----------------------------------------------------------------
//                          Widget classes


KoUnitDoubleSpinBox::KoUnitDoubleSpinBox( QWidget *parent, const char *name )
    : KDoubleSpinBox( parent ), KoUnitDoubleBase( KoUnit::U_PT, 2 )
    , m_lowerInPoints( -9999 )
    , m_upperInPoints( 9999 )
    , m_stepInPoints( 1 )
{
	setObjectName(name);
    KDoubleSpinBox::setPrecision( 2 );
    m_validator = new KoUnitDoubleValidator( this, this );
    setValidator( m_validator );
    setAcceptLocalizedNumbers( true );
    setUnit( KoUnit::U_PT );

    connect(this, SIGNAL(valueChanged( double )), SLOT(privateValueChanged()));
}


KoUnitDoubleSpinBox::KoUnitDoubleSpinBox( QWidget *parent,
						    double lower, double upper,
						    double step,
						    double value,
						    KoUnit::Unit unit,
						    unsigned int precision,
						    const char *name )
    : KDoubleSpinBox( lower, upper, step, value, parent, precision ),
      KoUnitDoubleBase( unit, precision ),
    m_lowerInPoints( lower ), m_upperInPoints( upper ), m_stepInPoints( step )
{
    m_unit = KoUnit::U_PT;
    m_validator = new KoUnitDoubleValidator( this, this );
    setValidator( m_validator );
    setAcceptLocalizedNumbers( true );
    setUnit( unit );
    changeValue( value );
    setLineStep( 0.5 );

    connect(this, SIGNAL(valueChanged( double )), SLOT(privateValueChanged()));
}

void
KoUnitDoubleSpinBox::changeValue( double val )
{
    KDoubleSpinBox::setValue( KoUnit::toUserValue( val, m_unit ) );
    // TODO: emit valueChanged ONLY if the value was out-of-bounds
    // This will allow the 'user' dialog to set a dirty bool and ensure
    // a proper value is getting saved.
}

void KoUnitDoubleSpinBox::privateValueChanged() {
    emit valueChangedPt( value () );
}

void
KoUnitDoubleSpinBox::setUnit( KoUnit::Unit unit )
{
    double oldvalue = KoUnit::fromUserValue( KDoubleSpinBox::value(), m_unit );
    KDoubleSpinBox::setMinimum( KoUnit::toUserValue( m_lowerInPoints, unit ) );
    KDoubleSpinBox::setMaximum( KoUnit::toUserValue( m_upperInPoints, unit ) );
    KDoubleSpinBox::setLineStep( KoUnit::toUserValue( m_stepInPoints, unit ) );
    KDoubleSpinBox::setValue( KoUnit::ptToUnit( oldvalue, unit ) );
    m_unit = unit;
    setSuffix( KoUnit::unitName( unit ).prepend( ' ' ) );
}

double KoUnitDoubleSpinBox::value( void ) const
{
    return KoUnit::fromUserValue( KDoubleSpinBox::value(), m_unit );
}

void KoUnitDoubleSpinBox::setMinimum( double min )
{
  m_lowerInPoints = min;
  KDoubleSpinBox::setMinimum( KoUnit::toUserValue( m_lowerInPoints, m_unit ) );
}

void KoUnitDoubleSpinBox::setMaximum( double max )
{
  m_upperInPoints = max;
  KDoubleSpinBox::setMaximum( KoUnit::toUserValue( m_upperInPoints, m_unit ) );
}

void KoUnitDoubleSpinBox::setLineStep( double step )
{
  m_stepInPoints = KoUnit::toUserValue(step, KoUnit::U_PT );
  KDoubleSpinBox::setLineStep( step );
}

void KoUnitDoubleSpinBox::setLineStepPt( double step )
{
  m_stepInPoints = step;
  KDoubleSpinBox::setLineStep( KoUnit::toUserValue( m_stepInPoints, m_unit ) );
}

void KoUnitDoubleSpinBox::setMinMaxStep( double min, double max, double step )
{
  setMinimum( min );
  setMaximum( max );
  setLineStepPt( step );
}

// ----------------------------------------------------------------


KoUnitDoubleLineEdit::KoUnitDoubleLineEdit( QWidget *parent, const char *name )
    : KLineEdit( parent ), KoUnitDoubleBase( KoUnit::U_PT, 2 ), m_value( 0.0 ), m_lower( 0.0 ), m_upper( 9999.99 ),
    m_lowerInPoints( 0.0 ), m_upperInPoints( 9999.99 )
{
    setAlignment( Qt::AlignRight );
    m_validator = new KoUnitDoubleValidator( this, this );
    setValidator( m_validator );
    setUnit( KoUnit::U_PT );
    changeValue(  KoUnit::ptToUnit( 0.0, KoUnit::U_PT ) );
}

KoUnitDoubleLineEdit::KoUnitDoubleLineEdit( QWidget *parent, double lower, double upper, double value, KoUnit::Unit unit,
    unsigned int precision, const char *name )
    : KLineEdit( parent ), KoUnitDoubleBase( unit, precision ), m_value( value ), m_lower( lower ), m_upper( upper ),
    m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    setAlignment( Qt::AlignRight );
    m_validator = new KoUnitDoubleValidator( this, this );
    setValidator( m_validator );
    setUnit( unit );
    changeValue(  KoUnit::ptToUnit( value, unit ) );
}

void
KoUnitDoubleLineEdit::changeValue( double value )
{
    m_value = value < m_lower ? m_lower : ( value > m_upper ? m_upper : value );
    setText( getVisibleText( m_value ) );
}

void
KoUnitDoubleLineEdit::setUnit( KoUnit::Unit unit )
{
    KoUnit::Unit old = m_unit;
    m_unit = unit;
    m_lower = KoUnit::ptToUnit( m_lowerInPoints, unit );
    m_upper = KoUnit::ptToUnit( m_upperInPoints, unit );
    changeValue( KoUnit::ptToUnit( KoUnit::fromUserValue( m_value, old ), unit ) );
}

bool
KoUnitDoubleLineEdit::eventFilter( QObject* o, QEvent* ev )
{
#if 0
	if( ev->type() == QEvent::FocusOut || ev->type() == QEvent::Leave || ev->type() == QEvent::Hide )
	{
		bool ok;
		double value = toDouble( text(), &ok );
		changeValue( value );
		return false;
	}
	else
#endif
            return QLineEdit::eventFilter( o, ev );
}

double KoUnitDoubleLineEdit::value( void ) const
{
    return KoUnit::fromUserValue( m_value, m_unit );
}


// ----------------------------------------------------------------


KoUnitDoubleComboBox::KoUnitDoubleComboBox( QWidget *parent, const char *name )
     : KComboBox( true, parent ), KoUnitDoubleBase( KoUnit::U_PT, 2 ), m_value( 0.0 ), m_lower( 0.0 ), m_upper( 9999.99 ), m_lowerInPoints( 0.0 ), m_upperInPoints( 9999.99 )
{
    lineEdit()->setAlignment( Qt::AlignRight );
    m_validator = new KoUnitDoubleValidator( this, this );
    lineEdit()->setValidator( m_validator );
    setUnit( KoUnit::U_PT );
    changeValue(  KoUnit::ptToUnit( 0.0, KoUnit::U_PT ) );
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
}

KoUnitDoubleComboBox::KoUnitDoubleComboBox( QWidget *parent, double lower, double upper, double value, KoUnit::Unit unit,
     unsigned int precision, const char *name )
     : KComboBox( true, parent ), KoUnitDoubleBase( unit, precision ), m_value( value ), m_lower( lower ), m_upper( upper ),
     m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    lineEdit()->setAlignment( Qt::AlignRight );
    m_validator = new KoUnitDoubleValidator( this, this );
    lineEdit()->setValidator( m_validator );
    setUnit( unit );
    changeValue(  KoUnit::ptToUnit( value, unit ) );
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
}

void
KoUnitDoubleComboBox::changeValue( double value )
{
    QString oldLabel = lineEdit()->text();
    updateValue( value );
    if( lineEdit()->text() != oldLabel )
        emit valueChanged( m_value );
}

void
KoUnitDoubleComboBox::updateValue( double value )
{
    m_value = value < m_lower ? m_lower : ( value > m_upper ? m_upper : value );
    lineEdit()->setText( getVisibleText( m_value ) );
}

void
KoUnitDoubleComboBox::insertItem( double value, int index )
{
    KComboBox::insertItem( getVisibleText( value ), index );
}

void
KoUnitDoubleComboBox::slotActivated( int index )
{
    double oldvalue = m_value;
    bool ok;
    double value = toDouble( text( index ), &ok );
    m_value = value < m_lower ? m_lower : ( value > m_upper ? m_upper : value );
    if( m_value != oldvalue )
        emit valueChanged( m_value );
}

void
KoUnitDoubleComboBox::setUnit( KoUnit::Unit unit )
{
    KoUnit::Unit old = m_unit;
    m_unit = unit;
    m_lower = KoUnit::ptToUnit( m_lowerInPoints, unit );
    m_upper = KoUnit::ptToUnit( m_upperInPoints, unit );
    changeValue( KoUnit::ptToUnit( KoUnit::fromUserValue( m_value, old ), unit ) );
}

bool
KoUnitDoubleComboBox::eventFilter( QObject* o, QEvent* ev )
{
#if 0
	if( ev->type() == QEvent::FocusOut || ev->type() == QEvent::Leave || ev->type() == QEvent::Hide )
	{
		bool ok;
		double value = toDouble( lineEdit()->text(), &ok );
		changeValue( value );
		return false;
	}
	else
#endif
            return QComboBox::eventFilter( o, ev );
}

double KoUnitDoubleComboBox::value( void ) const
{
    return KoUnit::fromUserValue( m_value, m_unit );
}


// ----------------------------------------------------------------


KoUnitDoubleSpinComboBox::KoUnitDoubleSpinComboBox( QWidget *parent, const char *name )
    : QWidget( parent ), m_step( 1.0 )
{
    Q3GridLayout *layout = new Q3GridLayout( this, 2, 3 );
    //layout->setMargin( 2 );
    QPushButton *up = new QPushButton( "+", this );
    //up->setFlat( true );
    up->setMaximumHeight( 15 );
    up->setMaximumWidth( 15 );
    layout->addWidget( up, 0, 0 );
    connect( up, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );

    QPushButton *down = new QPushButton( "-", this );
    down->setMaximumHeight( 15 );
    down->setMaximumWidth( 15 );
    layout->addWidget( down, 1, 0 );
    connect( down, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );

    m_combo = new KoUnitDoubleComboBox( this, KoUnit::ptToUnit( 0.0, KoUnit::U_PT ), KoUnit::ptToUnit( 9999.99, KoUnit::U_PT ), 0.0, KoUnit::U_PT, 2, name );
    connect( m_combo, SIGNAL( valueChanged( double ) ), this, SIGNAL( valueChanged( double ) ) );
    layout->addWidget( m_combo, 0, 2, 2, 1 );
}

KoUnitDoubleSpinComboBox::KoUnitDoubleSpinComboBox( QWidget *parent, double lower, double upper, double step, double value,
                                                    KoUnit::Unit unit, unsigned int precision, const char *name )
    : QWidget( parent ), m_step( step )//, m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    Q3GridLayout *layout = new Q3GridLayout( this, 2, 3 );
    //layout->setMargin( 2 );
    QPushButton *up = new QPushButton( "+", this );
    //up->setFlat( true );
    up->setMaximumHeight( 15 );
    up->setMaximumWidth( 15 );
    layout->addWidget( up, 0, 0 );
    connect( up, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );

    QPushButton *down = new QPushButton( "-", this );
    down->setMaximumHeight( 15 );
    down->setMaximumWidth( 15 );
    layout->addWidget( down, 1, 0 );
    connect( down, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );

    m_combo = new KoUnitDoubleComboBox( this, KoUnit::ptToUnit( lower, unit ), KoUnit::ptToUnit( upper, unit ), value, unit, precision, name );
    connect( m_combo, SIGNAL( valueChanged( double ) ), this, SIGNAL( valueChanged( double ) ) );
    layout->addWidget( m_combo, 0, 2, 2, 1 );
}

void
KoUnitDoubleSpinComboBox::slotUpClicked()
{
    m_combo->changeValue( m_combo->value() + m_step );
}

void
KoUnitDoubleSpinComboBox::slotDownClicked()
{
    m_combo->changeValue( m_combo->value() - m_step );
}

void
KoUnitDoubleSpinComboBox::insertItem( double value, int index )
{
    m_combo->insertItem( value, index );
}

void
KoUnitDoubleSpinComboBox::updateValue( double value )
{
    m_combo->updateValue( value );
}

double
KoUnitDoubleSpinComboBox::value() const
{
    return m_combo->value();
}

