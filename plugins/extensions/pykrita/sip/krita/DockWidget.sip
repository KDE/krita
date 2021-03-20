class DockWidget : public KDDockWidgets::DockWidget /NoDefaultCtors/
{
%TypeHeaderCode
#include "DockWidget.h"
%End

public:
    explicit DockWidget();
protected Q_SLOTS:
    Canvas* canvas() const;
    virtual void canvasChanged(Canvas *canvas) = 0;
    
};
