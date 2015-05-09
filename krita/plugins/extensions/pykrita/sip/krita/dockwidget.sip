%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class DockWidget : public QDockWidget /NoDefaultCtors/
{
%TypeHeaderCode
#include "dockwidget.h"
%End

public:
    DockWidget();

};
