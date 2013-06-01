#ifndef KIS_ANIMATION_FRAME_H
#define KIS_ANIMATION_FRAME_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QDebug>

class KisAnimationFrame : public QGraphicsItem
{
public:
    KisAnimationFrame(int x, int y, int width, int height, int type);
    ~KisAnimationFrame();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int getFrameType();
    int getY();
    int getX();
    KisAnimationFrame* getNextFrame();
    KisAnimationFrame* getPreviousFrame();
    void setNextFrame(KisAnimationFrame* nextFrame);
    void setPreviousFrame(KisAnimationFrame* previousFrame);

private:
    int m_x, m_y, m_width, m_height;
    int m_type;
    KisAnimationFrame* m_nextFrame;
    KisAnimationFrame* m_previousFrame;

public:
    const static int BLANKFRAME = 1;
    const static int KEYFRAME = 2;
    const static int SELECTEDFRAME = 0;
    const static int CONTINUEFRAME = 3;
};

#endif // KIS_ANIMATION_FRAME_H
