/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <math.h>

void FormRGBSliders::slotRedValueChanged( int v )
{
    this->textRedValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormRGBSliders::slotBlueValueChanged( int v )
{
  this->textBlueValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormRGBSliders::slotGreenValueChanged( int v )
{
  this->textGreenValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormRGBSliders::setMaxValue( int eV )
{
    this->sliderGreen->setMaxValue(eV);
    this->sliderRed->setMaxValue(eV);
    this->sliderBlue->setMaxValue(eV);
}


void FormRGBSliders::setMinValue( int eV )
{
    this->sliderGreen->setMinValue(eV);
    this->sliderRed->setMinValue(eV);
    this->sliderBlue->setMinValue(eV);
}


void FormRGBSliders::setPrecision( int eP )
{
    if( eP <= 0) return;
    this->precision = eP;
}


float FormRGBSliders::getGreenValue()
{
    return  this->sliderGreen->value()  / precision;
}


float FormRGBSliders::getBlueValue()
{
    return  this->sliderBlue->value()  / precision;
}


float FormRGBSliders::getRedValue()
{
    return  this->sliderRed->value()  / precision;
}


void FormRGBSliders::init()
{
    precision = 1;
}


void FormRGBSliders::setInitValue( int v )
{
    this->sliderRed->setValue(v * precision);
    this->sliderGreen->setValue(v * precision);
    this->sliderBlue->setValue(v * precision);
}


void FormRGBSliders::recalculTickInterval()
{
    int tickInterval = ( this->sliderBlue->maxValue() - this->sliderBlue->minValue() ) / 20;
    this->sliderRed->setTickInterval( tickInterval );
    this->sliderGreen->setTickInterval( tickInterval );
    this->sliderBlue->setTickInterval( tickInterval );
}
