#ifndef __GRADIENTDLG_H__
#define __GRADIENTDLG_H__

#include <kdialogbase.h>

class KisGradient;
class IntegerWidget;
class BlendChooser;
class QComboBox;

// the gradient tab widget can be inserted into the
// gradient dialog or any other configuration dialog

class GradientTab : public QWidget
{
	Q_OBJECT

public:

	GradientTab(KisGradient *_gradient,
        QWidget *_parent = 0, const char *_name = 0);

	int gradientOpacity()const;
	int gradientOffset()const;
	int gradientMode()const;
	int gradientBlend()const;
	int gradientType()const;
	int gradientRepeat()const;
	
private:

	IntegerWidget *opacity;
	IntegerWidget *offset;
	BlendChooser *mode;
	QComboBox *blend;
	QComboBox *gradient;
	QComboBox *repeat;
};


// a dialog specifically designed to hold the gradient widget

class GradientDialog : public KDialogBase
{
	Q_OBJECT

public:

	GradientDialog(KisGradient *_gradient,
		       QWidget *_parent = 0,
		       const char *_name = 0, 
		       bool modal = true);

	GradientTab *gradientTab ()const { return pGradientTab; }

private:

	GradientTab *pGradientTab;
};

#endif
