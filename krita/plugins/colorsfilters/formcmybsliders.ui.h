/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <math.h>

void FormCMYBSliders::slotCyanValueChanged( int v )
{
    this->textCyanValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormCMYBSliders::slotYellowValueChanged( int v )
{
  this->textYellowValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormCMYBSliders::slotMagentaValueChanged( int v )
{
  this->textMagentaValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}

void FormCMYBSliders::slotBlackValueChanged( int v )
{
  this->textBlackValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}

void FormCMYBSliders::setMaxValue( int eV )
{
    this->sliderMagenta->setMaxValue(eV);
    this->sliderCyan->setMaxValue(eV);
    this->sliderYellow->setMaxValue(eV);
}


void FormCMYBSliders::setMinValue( int eV )
{
    this->sliderMagenta->setMinValue(eV);
    this->sliderCyan->setMinValue(eV);
    this->sliderYellow->setMinValue(eV);
}


void FormCMYBSliders::setPrecision( int eP )
{
    if( eP <= 0) return;
    this->precision = eP;
}


float FormCMYBSliders::getMagentaValue()
{
    return  this->sliderMagenta->value()  / precision;
}


float FormCMYBSliders::getYellowValue()
{
    return  this->sliderYellow->value()  / precision;
}


float FormCMYBSliders::getCyanValue()
{
    this->sliderCyan->value() / precision;
}


float FormCMYBSliders::getBlackValue()
{
    this->sliderBlack->value() / precision;
}

void FormCMYBSliders::init()
{
    precision = 1;
}


void FormCMYBSliders::setInitValue( int v )
{
    this->sliderCyan->setValue(v * precision);
    this->sliderMagenta->setValue(v * precision);
    this->sliderYellow->setValue(v * precision);
}


void FormCMYBSliders::recalculTickInterval()
{
    int tickInterval = ( this->sliderYellow->maxValue() - this->sliderYellow->minValue() ) / 20;
    this->sliderCyan->setTickInterval( tickInterval );
    this->sliderMagenta->setTickInterval( tickInterval );
    this->sliderYellow->setTickInterval( tickInterval );
}
