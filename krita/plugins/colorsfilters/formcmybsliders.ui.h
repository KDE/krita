/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#define round(x) (int(float(x) + 0.5))

void FormCMYBSliders::slotCyanValueChanged( int v )
{
    textCyanValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormCMYBSliders::slotYellowValueChanged( int v )
{
  textYellowValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}


void FormCMYBSliders::slotMagentaValueChanged( int v )
{
  textMagentaValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}

void FormCMYBSliders::slotBlackValueChanged( int v )
{
  textBlackValue->setText(QString().setNum( round(10*(((float)v) / precision))/10 ));
}

void FormCMYBSliders::setMaxValue( int eV )
{
    sliderMagenta->setMaxValue(eV);
    sliderCyan->setMaxValue(eV);
    sliderYellow->setMaxValue(eV);
}


void FormCMYBSliders::setMinValue( int eV )
{
    sliderMagenta->setMinValue(eV);
    sliderCyan->setMinValue(eV);
    sliderYellow->setMinValue(eV);
}


void FormCMYBSliders::setPrecision( int eP )
{
    if( eP <= 0) return;
    precision = eP;
}


float FormCMYBSliders::getMagentaValue()
{
    return  sliderMagenta->value()  / precision;
}


float FormCMYBSliders::getYellowValue()
{
    return  sliderYellow->value()  / precision;
}


float FormCMYBSliders::getCyanValue()
{
    return sliderCyan->value() / precision;
}


float FormCMYBSliders::getBlackValue()
{
    return sliderBlack->value() / precision;
}

void FormCMYBSliders::init()
{
    precision = 1;
}


void FormCMYBSliders::setInitValue( int v )
{
    sliderCyan->setValue(v * precision);
    sliderMagenta->setValue(v * precision);
    sliderYellow->setValue(v * precision);
}


void FormCMYBSliders::recalculTickInterval()
{
    int tickInterval = ( sliderYellow->maxValue() - sliderYellow->minValue() ) / 20;
    sliderCyan->setTickInterval( tickInterval );
    sliderMagenta->setTickInterval( tickInterval );
    sliderYellow->setTickInterval( tickInterval );
}
