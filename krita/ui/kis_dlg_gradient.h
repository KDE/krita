#ifndef __GRADIENTDLG_H__
#define __GRADIENTDLG_H__

#include <kdialog.h>

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
    
    int gradientOpacity(); 
    int gradientOffset(); 
    int gradientMode();
    int gradientBlend(); 
    int gradientType(); 
    int gradientRepeat();
    
private:

    IntegerWidget *opacity;
    IntegerWidget *offset;
    BlendChooser *mode;
    QComboBox *blend;
    QComboBox *gradient;
    QComboBox *repeat;
};


// a dialog specifically designed to hold the gradient widget

class GradientDialog : public KDialog
{
    Q_OBJECT

public:

	GradientDialog(KisGradient *_gradient, 
        QWidget *_parent = 0, const char *_name = 0, bool modal = true);
    
    GradientTab *gradientTab () { return pGradientTab; }
    
private:

    GradientTab *pGradientTab;    
};

#endif
