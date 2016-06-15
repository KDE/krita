%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class DockWidget : public QDockWidget  /NoDefaultCtors/
{
%TypeHeaderCode
#include "dockwidget.h"
%End

public:
    explicit DockWidget();
protected:
    virtual void canvasChanged(Canvas *canvas) = 0;
};
