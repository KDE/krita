/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/




void KisWdgTextBrush::boldButtonClicked()
{
    if( radioButtonCustom->isChecked())
    {
	textLabelCustom->setEnabled(true);
	spinBoxCustomBoldness->setEnabled(true);
    } else {
	textLabelCustom->setEnabled(false);
        spinBoxCustomBoldness->setEnabled(false);
        if( radioButtonLight->isChecked() )
        {
	spinBoxCustomBoldness->setValue(25);
        }
        if( radioButtonNormal->isChecked() )
        {
	spinBoxCustomBoldness->setValue(50);
        }
        if( radioButtonBold->isChecked() )
        {
	spinBoxCustomBoldness->setValue(63);
        }
        if( radioButtonDemiBold->isChecked() )
        {
	spinBoxCustomBoldness->setValue(75);
        }
        if( radioButtonStrong->isChecked() )
        {
	spinBoxCustomBoldness->setValue(87);
        }
    }
}
