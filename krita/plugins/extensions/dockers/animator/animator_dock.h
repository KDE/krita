#ifndef _ANIMATOR_DOCK_H_
#define _ANIMATOR_DOCK_H_

#include <QDockWidget>
#include <QSpinBox>
#include <KoCanvasObserverBase.h>
#include "kis_timeline_view.h"

class QLabel;
class KisCanvas2;
class KisAnimation;

class AnimatorDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    AnimatorDock();
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas(){ m_canvas = 0;}

public slots:
    void updateNumberOfFrames();

private:
    KisTimelineView* m_timelineView;
    KisCanvas2 *m_canvas;
    KisAnimation *m_animation;
    QSpinBox *m_fpsInput;
    QSpinBox *m_timeInput;
};


#endif
