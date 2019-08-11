#ifndef KIS_MAINWINDOW_OBSERVER_H
#define KIS_MAINWINDOW_OBSERVER_H

#include <KoCanvasObserverBase.h>
#include <kritaui_export.h>

class KisViewManager;

/**
 * @brief The KisMainwindowObserver class is an interface for dock widgets
 * that want to keep track of the main window as well as the canvas.
 */
class KRITAUI_EXPORT KisMainwindowObserver : public KoCanvasObserverBase
{
public:
    KisMainwindowObserver();
    ~KisMainwindowObserver() override;

    virtual void setViewManager(KisViewManager* kisview) = 0;

};

#endif // KIS_MAINWINDOW_OBSERVER_H
