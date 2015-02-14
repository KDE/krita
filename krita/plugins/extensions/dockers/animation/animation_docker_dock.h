
#ifndef _ANIMATION_DOCK_H_
#define _ANIMATION_DOCK_H_

#include <QDockWidget>

#include <KoCanvasObserverBase.h>

class KisCanvas2;
class Ui_WdgAnimation;

class AnimationDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    AnimationDockerDock();
    QString observerName() { return "AnimationDockerDock"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

private slots:
    void slotAddBlankFrame();
    void slotPreviousFrame();
    void slotNextFrame();

private:

    KisCanvas2 *m_canvas;
    Ui_WdgAnimation *m_animationWidget;
};


#endif
