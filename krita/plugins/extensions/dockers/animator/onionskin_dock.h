#ifndef ONIONSKIN_DOCK_H
#define ONIONSKIN_DOCK_H

#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#include <QCheckBox>
#include <QSpinBox>
#include <KColorButton>
#include "kis_opacity_selector_view.h"
class KisCanvas2;
class KisAnimation;

class OnionSkinDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    OnionSkinDock();
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas(){ m_canvas = 0;}

private:
    KisCanvas2* m_canvas;
    KisAnimation* m_animation;
    QCheckBox *m_activeCheckBox;
    QSpinBox* m_previousFramesInput;
    QSpinBox* m_nextFramesInput;
    KColorButton* m_previousFramesColor;
    KColorButton* m_nextFramesColor;
    KisOpacitySelectorView* m_previousOpacitySelectorView;
    KisOpacitySelectorView* m_nextOpacitySelectorView;
};

#endif // ONIONSKIN_DOCK_H
