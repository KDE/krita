#ifndef KIS_TIMELINE_VIEW_H
#define KIS_TIMELINE_VIEW_H

#include <QGraphicsView>
#include <QList>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QDebug>
#include <math.h>
#include <QKeyEvent>
#include "kis_timeline.h"
#include "kis_animation_frame.h"

class KisTimelineView : public QGraphicsView
{
    Q_OBJECT
public:
    KisTimelineView(QWidget* parent);
    ~KisTimelineView();
    QGraphicsScene *m_timelineScene;
    KisTimeline* m_tl;
    QSize sizeHint() const;
    void init();

private:
    void removePreviousSelection();
    KisAnimationFrame* m_selectedFrame;

public:
    int m_numberOfFrames;

public slots:
    void setNumberOfFrames(int val);
    void addKeyFrame();
    void addBlankFrame();

protected:
    void mousePressEvent(QMouseEvent *event);

signals:

};

#endif // KIS_TIMELINE_VIEW_H
