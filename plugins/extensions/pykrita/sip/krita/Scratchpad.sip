class Scratchpad : public QWidget /NoDefaultCtors/
{
%TypeHeaderCode
#include "Scratchpad.h"
%End

public:
    Scratchpad(View* view , const QString & defaultColor, QWidget*  parent /TransferThis/ = 0);	
		virtual ~Scratchpad();

public Q_SLOTS:
    void clear();
    void setModeManually(bool value);
    void setMode(QString modeName);
};
