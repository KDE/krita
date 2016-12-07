class DockWidget : public QDockWidget /NoDefaultCtors/
{
%TypeHeaderCode
#include "DockWidget.h"
%End

public:
    explicit DockWidget();
protected Q_SLOTS:
    virtual void canvasChanged(Canvas *canvas) = 0;
    
};
