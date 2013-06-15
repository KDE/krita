#ifndef KIS_TIMELINE_H
#define KIS_TIMELINE_H

#include <QWidget>
#include <QScrollBar>
#include <QToolButton>

class KisTimelineCells;
class KisAnimationLayerBox;
class KisCanvas2;

class KisTimeline : public QWidget
{
    Q_OBJECT
public:
    KisTimeline(QWidget* parent = 0);
    void setCanvas(KisCanvas2* canvas);
    KisCanvas2* getCanvas();
    KisAnimationLayerBox* getLayerBox();
    QScrollBar *m_hScrollBar, *m_vScrollBar;
    bool scrubbing;

protected:
    void resizeEvent(QResizeEvent *event);

public:
    QToolButton* m_addLayerButton;

private:
    KisTimelineCells* m_cells;
    int m_numberOfLayers;
    KisCanvas2* m_canvas;
    KisAnimationLayerBox *m_list;
};

#endif // KIS_TIMELINE_H
