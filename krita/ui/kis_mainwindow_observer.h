#ifndef KIS_MAINWINDOW_OBSERVER_H
#define KIS_MAINWINDOW_OBSERVER_H

#include <KoCanvasObserverBase.h>
#include <krita_export.h>

class KisViewManager;

class KRITAUI_EXPORT KisMainwindowObserver : public KoCanvasObserverBase
{   
public:
    KisMainwindowObserver();
    virtual ~KisMainwindowObserver();

    virtual void setMainWindow(KisViewManager* kisview) = 0;
    virtual void setCanvas(KoCanvasBase* canvas);
    virtual void unsetCanvas();
};

#endif // KIS_MAINWINDOW_OBSERVER_H
