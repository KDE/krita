/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#define round(x) (int(float(x) + 0.5))

void FormRGBSliders::slotRedValueChanged( int v )
{
    textRedValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormRGBSliders::slotBlueValueChanged( int v )
{
  textBlueValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormRGBSliders::slotGreenValueChanged( int v )
{
  textGreenValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormRGBSliders::setMaxValue( int eV )
{
    sliderGreen->setMaxValue(eV);
    sliderRed->setMaxValue(eV);
    sliderBlue->setMaxValue(eV);
}


void FormRGBSliders::setMinValue( int eV )
{
    sliderGreen->setMinValue(eV);
    sliderRed->setMinValue(eV);
    sliderBlue->setMinValue(eV);
}


void FormRGBSliders::setPrecision( int eP )
{
    if( eP <= 0) return;
    precision = eP;
}


float FormRGBSliders::getGreenValue()
{
    return  sliderGreen->value()  / precision;
}


float FormRGBSliders::getBlueValue()
{
    return  sliderBlue->value()  / precision;
}


float FormRGBSliders::getRedValue()
{
    return  sliderRed->value()  / precision;
}


void FormRGBSliders::init()
{
    precision = 1;
}


void FormRGBSliders::setInitValue( int v )
{
    sliderRed->setValue(v * precision);
    sliderGreen->setValue(v * precision);
    sliderBlue->setValue(v * precision);
}


void FormRGBSliders::recalculTickInterval()
{
    int tickInterval = ( sliderBlue->maxValue() - sliderBlue->minValue() ) / 20;
    sliderRed->setTickInterval( tickInterval );
    sliderGreen->setTickInterval( tickInterval );
    sliderBlue->setTickInterval( tickInterval );
}
