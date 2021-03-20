class DockWidgetFactoryBase
{
%TypeHeaderCode
#include "DockWidgetFactoryBase.h"
%End

public:
    enum DockPosition {
        DockTornOff, ///< Floating as its own top level window
        DockTop,    ///< Above the central widget
        DockBottom, ///< Below the central widget
        DockRight,  ///< Right of the centra widget
        DockLeft,   ///< Left of the centra widget
        DockMinimized  ///< Not docked, but reachable via the menu
    };
    DockWidgetFactoryBase(const QString& _id, DockPosition _dockPosition);
    virtual QDockWidget* createDockWidget() = 0 /Factory/;
    QString id() const;
};

